#include "Search/iSax/iSaxEnvelopeIndex.hpp"
#include "Search/Index.hpp"
#include "Search/iSax/iSaxSplittableNode.hpp"
#include "Search/iSax/iSaxSplitStrategy.hpp"
#include "Search/iSax/iSaxEnvelopeFinalizedIndex.hpp"
#include "Summarization/Envelope.hpp"
#include "Summarization/iSaxWord.hpp"
#include "Summarization/iSaxBreakpointStrategy.hpp"
#include "Util/typedefs.hpp"
#include "Util/Logger.hpp"
#include "Util/RunSettings.hpp"

std::size_t SaxSymbolsHash::operator()(const vec<vec<SaxSymbolT>> &symbols) const {
    std::size_t seed = 0, num_symbols = symbols[0].size();
    for (auto channel : symbols) {
        for (SaxSymbolT symbol : channel) {
            boost::hash_combine(seed, symbol);
        }
    }
    return seed;
}

iSaxEnvelopeIndex::iSaxEnvelopeIndex(const SeriesISaxProperties &series_isax_prop, SaxNumBitsT first_layer_num_bits,
                                     size_t leaf_capacity, std::unique_ptr<IiSaxSplitStrategy> split_strategy)
    : m_segment_len(series_isax_prop.segment_len),
      m_series_len(series_isax_prop.series_len),
      m_pos_per_env(series_isax_prop.pos_per_env),
      m_num_channels(series_isax_prop.num_channels),
      m_num_seg_per_channel(series_isax_prop.num_seg_per_channel),
      m_first_layer_num_bits(first_layer_num_bits),
      m_leaf_capacity(leaf_capacity),
      m_split_strategy(std::move(split_strategy)) {
    assert(m_segment_len > 0);
    assert(m_num_channels > 0);
    assert(first_layer_num_bits > 0);

    auto &RS = RunSettings::get_instance();
    m_alphabet_num_bits = RS.get_isax_props().m_breakpoint_num_bits;
    m_breakpoints = &RS.get_breakpoints();

    assert(m_breakpoints->size() == (1 << m_alphabet_num_bits) - 1);
    assert(m_alphabet_num_bits >= m_first_layer_num_bits);
}

void iSaxEnvelopeIndex::split_leaf(vec<iSaxWord> &isax_mins, std::unique_ptr<iSaxSplittableNode> &node_ref) {
    auto *leaf = static_cast<iSaxSplittableLeaf *>(node_ref.get());
    // Split the leaf
    auto [segment_ind, channel_ind] = m_split_strategy->get_split_ind(leaf, isax_mins);
    SaxNumBitsT split_seg_bits = isax_mins[channel_ind].get_num_bits()[segment_ind];
    // If cannot split further, return
    if (split_seg_bits == m_alphabet_num_bits) return;

    auto &logger = IndexLogger::get_instance();
    logger.increment_count_col(ISC::NUM_NODES, 2);
    logger.increment_count_col(ISC::NUM_LEAVES);

    // Find the breakpoint in the middle of the symbol at the split index
    SaxSymbolT alphabet_size_ratio = (m_breakpoints->size() + 1) / (1 << split_seg_bits);

    // `symbol * 2 + 1` goes to the upper interval in the next resolution
    // `* (alphabet_size_ratio >> 1)` goes to the lowest portion of the upper interval
    // (i.e. just above the mid breakpoint) in the desired resolution
    // `-1` adjusts for the fact that the breakpoints have an implicit -inf at the beginning
    auto symbol = isax_mins[channel_ind][segment_ind];
    auto mid_breakpoint = m_breakpoints->at((symbol * 2 + 1) * (alphabet_size_ratio >> 1) - 1);

    // Distribute the mts_envelope across the two new leaves
    vec<SubsequencePosition> left_subsequence_positions, right_subsequence_positions;
    vec<vec<Envelope>> left_mts_envelope, right_mts_envelope;

    for (size_t i = 0; i < leaf->m_subsequence_positions.size(); ++i) {
        auto &seg_min = leaf->m_envelopes[i][channel_ind].lower;
        if (seg_min[segment_ind] <= mid_breakpoint) {
            left_subsequence_positions.push_back(leaf->m_subsequence_positions[i]);
            left_mts_envelope.push_back(leaf->m_envelopes[i]);
        } else {
            right_subsequence_positions.push_back(leaf->m_subsequence_positions[i]);
            right_mts_envelope.push_back(leaf->m_envelopes[i]);
        }
    }

    // Create new nodes
    size_t left_size = left_subsequence_positions.size(), right_size = right_subsequence_positions.size();
    auto new_internal = std::make_unique<iSaxSplittableInternal>(SaxSplitIndex{segment_ind, channel_ind});
    new_internal->m_left =
        std::make_unique<iSaxSplittableLeaf>(std::move(left_subsequence_positions), std::move(left_mts_envelope));
    new_internal->m_right =
        std::make_unique<iSaxSplittableLeaf>(std::move(right_subsequence_positions), std::move(right_mts_envelope));

    // Replace the leaf with the new internal node
    node_ref = std::move(new_internal);

    // Check if further splitting is necessary
    bool split_left = left_size > m_leaf_capacity, split_right = right_size > m_leaf_capacity;
    if (split_left || split_right) {
        auto &parent = reinterpret_cast<std::unique_ptr<iSaxSplittableInternal> &>(node_ref);
        if (split_left) {
            uint8_t new_bit = 0;
            isax_mins[channel_ind].append_to_symbol(segment_ind, new_bit);
            leaf = static_cast<iSaxSplittableLeaf *>(parent->m_left.get());
            split_leaf(isax_mins, parent->m_left);
            isax_mins[channel_ind].remove_from_symbol(segment_ind);
        }
        if (split_right) {
            uint8_t new_bit = 1;
            isax_mins[channel_ind].append_to_symbol(segment_ind, new_bit);
            leaf = static_cast<iSaxSplittableLeaf *>(parent->m_right.get());
            split_leaf(isax_mins, parent->m_right);
            isax_mins[channel_ind].remove_from_symbol(segment_ind);
        }
    }
}

void iSaxEnvelopeIndex::insert(const IndexEntry<Envelope> &entry) {
    const vec<Envelope> &mts_envelope = entry.mts_summary;
    SubsequencePosition file_pos = entry.subsequence_position;

    assert(mts_envelope.size() == m_num_channels);
    assert(mts_envelope[0].size() == m_num_seg_per_channel);

    vec<iSaxWord> isax_mins(m_num_channels);
    vec<vec<SaxSymbolT>> symbols(m_num_channels, vec<SaxSymbolT>(m_num_seg_per_channel));
    for (MtsNumChannelsT c = 0; c < m_num_channels; ++c) {
        auto env_min = mts_envelope[c].lower;
        isax_mins[c] = iSaxWord(
            env_min, {vec<SaxNumBitsT>(env_min.size(), m_first_layer_num_bits), m_alphabet_num_bits, *m_breakpoints});
        for (SaxSegIndT s = 0; s < m_num_seg_per_channel; ++s) symbols[c][s] = isax_mins[c][s];
    }

    auto node_it = m_first_layer.find(symbols);
    if (node_it == m_first_layer.end()) {
        m_first_layer.emplace(symbols, std::make_unique<iSaxSplittableLeaf>(vec<SubsequencePosition>{file_pos},
                                                                            vec<vec<Envelope>>{mts_envelope}));

        auto &logger = IndexLogger::get_instance();
        logger.increment_count_col(ISC::NUM_NODES);
        logger.increment_count_col(ISC::NUM_LEAVES);
    } else {
        auto node = node_it->second.get();
        iSaxSplittableInternal *parent = nullptr;
        uint8_t new_bit = 0;
        // Traverse tree until a leaf is reached
        while (!(node->is_leaf())) {
            parent = static_cast<iSaxSplittableInternal *>(node);
            auto [segment_ind, channel_ind] = node->get_split_ind();
            new_bit = isax_mins[channel_ind].apply_split(segment_ind);
            node = const_cast<iSaxSplittableNode *>(new_bit ? node->get_children().second : node->get_children().first);
        }
        // Reached a leaf => insert
        auto *leaf = static_cast<iSaxSplittableLeaf *>(node);
        leaf->m_subsequence_positions.push_back(file_pos);
        leaf->m_envelopes.push_back(mts_envelope);

        // Split if needed
        if (leaf->m_subsequence_positions.size() > m_leaf_capacity) {
            auto &node_ref = parent ? (new_bit ? parent->m_right : parent->m_left) : node_it->second;
            split_leaf(isax_mins, node_ref);
        }
    }
}

std::unique_ptr<IFinalizedIndex<Envelope>> iSaxEnvelopeIndex::finalize() {
    size_t size_first_layer = m_first_layer.size();
    vec<vec<vec<SaxSymbolT>>> first_layer_min_symbols(size_first_layer), first_layer_max_symbols(size_first_layer);
    vec<std::unique_ptr<iSaxFinalizedNode>> finalized_nodes(size_first_layer);

    iSaxWordSettings isax_word_settings = {vec<SaxNumBitsT>(m_num_seg_per_channel, m_alphabet_num_bits),
                                           m_alphabet_num_bits, *m_breakpoints};

    size_t i = 0;
    for (auto it = m_first_layer.begin(); it != m_first_layer.end(); ++it) {
        const auto &min_symbols = it->first;
        auto &node = it->second;

        auto [finalized_node, isax_max] = node->finalize(isax_word_settings);
        vec<vec<SaxSymbolT>> max_symbols(m_num_channels, vec<SaxSymbolT>(m_num_seg_per_channel));

        SaxNumBitsT shift = m_alphabet_num_bits - m_first_layer_num_bits;
        for (MtsNumChannelsT c = 0; c < m_num_channels; ++c) {
            for (SaxSegIndT s = 0; s < m_num_seg_per_channel; ++s)
                max_symbols[c][s] = isax_max[c].symbol_no_shift(s) >> shift;
        }
        first_layer_min_symbols[i] = min_symbols;
        first_layer_max_symbols[i] = max_symbols;
        finalized_nodes[i] = std::move(finalized_node);
        ++i;

        node.reset();
    }

    SeriesISaxProperties series_isax_prop = {m_segment_len, m_series_len, m_pos_per_env, m_num_channels,
                                             m_num_seg_per_channel};
    return std::make_unique<iSaxEnvelopeFinalizedIndex>(series_isax_prop, std::move(first_layer_min_symbols),
                                                        std::move(first_layer_max_symbols), std::move(finalized_nodes),
                                                        m_first_layer_num_bits, m_alphabet_num_bits, *m_breakpoints);
}

const iSaxSplittableNode *iSaxEnvelopeIndex::get_first_layer_node(const vec<iSaxWord> &isax_mins) const {
    vec<vec<SaxSymbolT>> symbols(m_num_channels, vec<SaxSymbolT>(m_num_seg_per_channel));
    for (MtsNumChannelsT c = 0; c < m_num_channels; ++c) {
        for (SaxSegIndT s = 0; s < m_num_seg_per_channel; ++s) symbols[c][s] = isax_mins[c][s];
    }
    auto node_it = m_first_layer.find(symbols);
    return node_it == m_first_layer.end() ? nullptr : node_it->second.get();
}

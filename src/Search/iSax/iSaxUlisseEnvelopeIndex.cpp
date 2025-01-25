#include "Search/iSax/iSaxUlisseEnvelopeIndex.hpp"
#include "Search/iSax/iSaxFinalizedUliEnvIndex.hpp"
#include "Search/iSax/iSaxSplittableNode.hpp"
#include "Summarization/iSaxWord.hpp"

#include <iostream>

std::size_t iSaxWordVecHash::operator()(const vec<iSaxWord> &isax_mins) const {
    std::size_t seed = 0, num_symbols = isax_mins[0].size();
    for (auto isax_min : isax_mins) {
        for (size_t i = 0; i < num_symbols; ++i) {
            boost::hash_combine(seed, isax_min[i]);
        }
    }
    return seed;
}

iSaxUlisseEnvelopeIndex::iSaxUlisseEnvelopeIndex(SaxSegIndT num_seg_per_channel, MtsNumChannelsT num_channels,
                                                 SaxNumBitsT first_layer_num_bits, size_t leaf_capacity,
                                                 std::unique_ptr<IiSaxBreakpointStrategy> breakpoint_strategy,
                                                 std::unique_ptr<IiSaxSplitStrategy> split_strategy,
                                                 SaxNumBitsT num_bits_limit)
    : m_num_seg_per_channel(num_seg_per_channel),
      m_num_channels(num_channels),
      m_first_layer_num_bits(first_layer_num_bits),
      m_alphabet_num_bits(first_layer_num_bits),
      m_num_bits_limit(num_bits_limit),
      m_leaf_capacity(leaf_capacity),
      m_breakpoint_strategy(std::move(breakpoint_strategy)),
      m_split_strategy(std::move(split_strategy)),
      m_breakpoints(m_breakpoint_strategy->get_breakpoints(1 << m_alphabet_num_bits)) {
    assert(num_seg_per_channel > 0);
    assert(num_channels > 0);
    assert(first_layer_num_bits > 0);
    assert(num_bits_limit >= first_layer_num_bits);
}

void iSaxUlisseEnvelopeIndex::split_leaf(vec<iSaxWord> &isax_mins, const vec<UlisseEnvelope> &envelopes,
                                         std::unique_ptr<iSaxSplittableNode> &node_ref) {
    // Split the leaf
    auto [segment_ind, channel_ind] = m_split_strategy->get_split_ind();
    SaxNumBitsT split_seg_bits = isax_mins[channel_ind].get_num_bits()[segment_ind];
    // If cannot split further, return
    if (split_seg_bits == m_num_bits_limit) return;

    // // The alphabet size was doubled, update the iSAX word
    // isax_mins = iSaxWord(env_min, isax_mins.get_num_bits(), m_alphabet_num_bits, m_breakpoints);
    // isax_update_required = false;

    // Find the breakpoint in the middle of the symbol at the split index
    SaxSymbolT breakpoint_alphabet_size = (m_breakpoints.size() + 1);
    SaxSymbolT alphabet_size_ratio = breakpoint_alphabet_size / (1 << split_seg_bits);

    bool isax_update_required = false;
    // If the segment already used the full alphabet, double the alphabet size
    // Do not update the iSAX word yet, it may not be necessary
    if (alphabet_size_ratio == 1) {
        m_breakpoints = m_breakpoint_strategy->get_breakpoints(breakpoint_alphabet_size * 2);
        alphabet_size_ratio = 2;
        ++m_alphabet_num_bits;
        isax_update_required = true;
    }
    // `symbol * 2 + 1` goes to the upper interval in the next resolution
    // `* (alphabet_size_ratio >> 1)` goes to the lowest portion of the upper interval
    // (i.e. just above the mid breakpoint) in the desired resolution
    // `-1` adjusts for the fact that the breakpoints have an implicit -inf at the beginning
    auto symbol = isax_mins[channel_ind][segment_ind];
    auto mid_breakpoint = m_breakpoints[(symbol * 2 + 1) * (alphabet_size_ratio >> 1) - 1];

    // Distribute the envelopes across the two new leaves
    vec<FilePositionT> left_file_positions, right_file_positions;
    vec<vec<UlisseEnvelope>> left_envelopes, right_envelopes;

    auto *leaf = static_cast<iSaxSplittableLeaf *>(node_ref.get());
    for (size_t i = 0; i < leaf->m_file_positions.size(); ++i) {
        auto &seg_min = leaf->m_envelopes[i][channel_ind].first;
        if (seg_min[segment_ind] <= mid_breakpoint) {
            left_file_positions.push_back(leaf->m_file_positions[i]);
            left_envelopes.push_back(leaf->m_envelopes[i]);
        } else {
            right_file_positions.push_back(leaf->m_file_positions[i]);
            right_envelopes.push_back(leaf->m_envelopes[i]);
        }
    }

    // Create new nodes
    size_t left_size = left_file_positions.size(), right_size = right_file_positions.size();
    auto new_internal = std::make_unique<iSaxSplittableInternal>(SaxSplitIndT{segment_ind, channel_ind});
    new_internal->m_left =
        std::make_unique<iSaxSplittableLeaf>(std::move(left_file_positions), std::move(left_envelopes));
    new_internal->m_right =
        std::make_unique<iSaxSplittableLeaf>(std::move(right_file_positions), std::move(right_envelopes));

    // Replace the leaf with the new internal node
    node_ref = std::move(new_internal);

    // Check if further splitting is necessary
    bool split_left = left_size > m_leaf_capacity, split_right = right_size > m_leaf_capacity;
    if (split_left || split_right) {
        if (isax_update_required) {
            for (MtsNumChannelsT c = 0; c < m_num_channels; ++c) {
                isax_mins[c] =
                    iSaxWord(envelopes[c].first, {isax_mins[c].get_num_bits(), m_alphabet_num_bits, m_breakpoints});
            }
        }
        auto &parent = reinterpret_cast<std::unique_ptr<iSaxSplittableInternal> &>(node_ref);

        if (split_left) {
            uint8_t new_bit = 0;
            isax_mins[channel_ind].append_to_symbol(segment_ind, new_bit);
            leaf = static_cast<iSaxSplittableLeaf *>(parent->m_left.get());
            split_leaf(isax_mins, envelopes, parent->m_left);
            isax_mins[channel_ind].remove_from_symbol(segment_ind);
        }
        if (split_right) {
            uint8_t new_bit = 1;
            isax_mins[channel_ind].append_to_symbol(segment_ind, new_bit);
            leaf = static_cast<iSaxSplittableLeaf *>(parent->m_right.get());
            split_leaf(isax_mins, envelopes, parent->m_right);
            isax_mins[channel_ind].remove_from_symbol(segment_ind);
        }
    }
}

void iSaxUlisseEnvelopeIndex::insert(const vec<UlisseEnvelope> &envelopes, FilePositionT file_pos) {
    assert(envelopes.size() == m_num_channels);
    assert(envelopes[0].first.size() == m_num_seg_per_channel);

    vec<iSaxWord> isax_mins(envelopes.size());
    for (size_t i = 0; i < envelopes.size(); ++i) {
        auto env_min = envelopes[i].first;
        isax_mins[i] = iSaxWord(
            env_min, {vec<SaxNumBitsT>(env_min.size(), m_first_layer_num_bits), m_alphabet_num_bits, m_breakpoints});
    }

    auto node_it = m_first_layer.find(isax_mins);
    if (node_it == m_first_layer.end()) {
        m_first_layer.emplace(isax_mins, std::make_unique<iSaxSplittableLeaf>(vec<FilePositionT>{file_pos},
                                                                              vec<vec<UlisseEnvelope>>{envelopes}));
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
        leaf->m_file_positions.push_back(file_pos);
        leaf->m_envelopes.push_back(envelopes);

        // Split if needed
        if (leaf->m_file_positions.size() > m_leaf_capacity) {
            auto &node_ref = parent ? (new_bit ? parent->m_right : parent->m_left) : node_it->second;
            split_leaf(isax_mins, envelopes, node_ref);
        }
    }
}

std::unique_ptr<IFinalizedUliEnvIndex> iSaxUlisseEnvelopeIndex::finalize() {
    // I. Go over each entry in `m_first_layer_nodes`
    //     1. `auto [new_node, isax_max] = node->finalize()`; PROBLEM: `iSaxInternal` will have a `finalize` method
    //     2. Set `node = new_node`
    //     3. Save `isax_max` into the `m_first_layer_isax_max` vector
    // `node->finalize()`:
    return std::make_unique<iSaxFinalizedUliEnvIndex>();
}

vec<FilePositionT> iSaxUlisseEnvelopeIndex::search(vec<vec<float>> mts, const SearchOptions &search_options) const {
    return {};
};

const iSaxSplittableNode *iSaxUlisseEnvelopeIndex::get_first_layer_node(const vec<iSaxWord> &isax_mins) const {
    auto node_it = m_first_layer.find(isax_mins);
    return node_it == m_first_layer.end() ? nullptr : node_it->second.get();
}

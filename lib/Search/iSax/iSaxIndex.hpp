#ifndef ISAX_INDEX_HPP
#define ISAX_INDEX_HPP

#include <unordered_map>

#include "Util/typedefs.hpp"
#include "Search/Index.hpp"
#include "Search/iSax/iSaxSplittableNode.hpp"
#include "Search/iSax/iSaxSplitStrategy.hpp"
#include "Search/iSax/iSaxFinalizedIndex.hpp"
#include "Summarization/Envelope.hpp"
#include "Summarization/iSaxWord.hpp"
#include "Summarization/iSaxBreakpointStrategy.hpp"
#include "Summarization/Paa.hpp"

struct SaxSymbolsHash {
    std::size_t operator()(const vec<vec<SaxSymbolT>> &symbols) const;
};

/**
 * @brief iSAX index
 * @tparam T Type data stored in the index
 */
template <typename T>
    requires DerivedFromEntryData<T>
class iSaxIndex : public IIndex<T> {
    using FTag = typename iSaxIndexTraits<T>::FinalizedTag;
    using SymbolType = typename SaxTraits<FTag>::SymbolType;

   public:
    /**
     * @brief Construct a new iSaxIndex object
     *
     * @param series_isax_prop Properties of the time series
     * @param first_layer_num_bits Number of bits used for symbols in the first layer
     * @param leaf_capacity Capacity of the leaf nodes
     * @param split_strategy Split strategy
     */
    iSaxIndex(uptr<SeriesISaxProperties> series_isax_prop, SaxNumBitsT first_layer_num_bits, size_t leaf_capacity,
              uptr<IiSaxSplitStrategy<T>> split_strategy)
        : m_series_isax_prop(std::move(series_isax_prop)),
          m_first_layer_num_bits(first_layer_num_bits),
          m_leaf_capacity(leaf_capacity),
          m_split_strategy(std::move(split_strategy)) {
        assert(m_series_isax_prop->segment_len > 0);
        assert(m_series_isax_prop->num_channels > 0);
        assert(first_layer_num_bits > 0);

        auto &RS = RunSettings::get_instance();
        m_alphabet_num_bits = RS.get_isax_props().m_breakpoint_num_bits;
        m_breakpoints = &RS.get_breakpoints();

        assert(m_breakpoints->size() == (1 << m_alphabet_num_bits) - 1);
        assert(m_alphabet_num_bits >= m_first_layer_num_bits);
    }

    iSaxIndex() = default;

    ~iSaxIndex() = default;

    void insert(const IndexEntry<T> &entry) override {
        const vec<T> &mts_summary = entry.mts_summary;
        SubsequenceInfo file_pos = entry.subsequence_position;

        MtsNumChannelsT num_channels = m_series_isax_prop->num_channels;
        SaxSegIndT num_seg_per_channel = m_series_isax_prop->num_seg_per_channel;

        assert(mts_summary.size() == num_channels);
        assert(mts_summary[0].size() == num_seg_per_channel);

        vec<iSaxWord> isax_words(num_channels);
        vec<vec<SaxSymbolT>> symbols(num_channels, vec<SaxSymbolT>(num_seg_per_channel));
        for (MtsNumChannelsT c = 0; c < num_channels; ++c) {
            auto isax_input = mts_summary[c].get_isax_input();
            isax_words[c] = iSaxWord(isax_input, {vec<SaxNumBitsT>(isax_input.size(), m_first_layer_num_bits),
                                                  m_alphabet_num_bits, *m_breakpoints});
            for (SaxSegIndT s = 0; s < num_seg_per_channel; ++s) symbols[c][s] = isax_words[c][s];
        }

        auto node_it = m_first_layer.find(symbols);
        if (node_it == m_first_layer.end()) {
            // TODO: either function to generate pointers or second / third template parameter
            m_first_layer.emplace(symbols, std::make_unique<iSaxSplittableLeaf<T>>(vec<SubsequenceInfo>{file_pos},
                                                                                   vec<vec<T>>{mts_summary}));

            auto &logger = IndexLogger::get_instance();
            logger.increment_count_col(ISC::NUM_NODES);
            logger.increment_count_col(ISC::NUM_LEAVES);
        } else {
            auto node = node_it->second.get();
            iSaxSplittableInternal<T> *parent = nullptr;
            uint8_t new_bit = 0;
            // Traverse tree until a leaf is reached
            while (!(node->is_leaf())) {
                parent = static_cast<iSaxSplittableInternal<T> *>(node);
                auto [segment_ind, channel_ind] = node->get_split_ind();
                new_bit = isax_words[channel_ind].apply_split(segment_ind);
                node = const_cast<iSaxSplittableNode<T> *>(new_bit ? node->get_children().second
                                                                   : node->get_children().first);
            }
            // Reached a leaf => insert
            auto *leaf = static_cast<iSaxSplittableLeaf<T> *>(node);
            leaf->m_subsequence_positions.push_back(file_pos);
            leaf->m_summaries.push_back(mts_summary);

            // Split if needed
            if (leaf->m_subsequence_positions.size() > m_leaf_capacity) {
                auto &node_ref = parent ? (new_bit ? parent->m_right : parent->m_left) : node_it->second;
                split_leaf(isax_words, node_ref);
            }
        }
    }

    uptr<IFinalizedIndex<FTag>> finalize() override {
        size_t size_first_layer = m_first_layer.size();
        vec<vec<vec<SymbolType>>> first_layer_symbols(size_first_layer);
        vec<uptr<iSaxFinalizedNode<FTag>>> finalized_nodes(size_first_layer);

        iSaxWordSettings isax_word_settings = {
            vec<SaxNumBitsT>(m_series_isax_prop->num_seg_per_channel, m_alphabet_num_bits),
            m_alphabet_num_bits,
            *m_breakpoints,
        };

        size_t i = 0;
        for (auto it = m_first_layer.begin(); it != m_first_layer.end(); ++it) {
            const auto &key_symbols = it->first;
            auto &node = it->second;

            auto [finalized_node, isax_symbols] = finalize_first_layer_node(key_symbols, node, isax_word_settings);
            first_layer_symbols[i] = isax_symbols;
            // first_layer_min_symbols[i] = min_symbols;
            // first_layer_max_symbols[i] = max_symbols;
            finalized_nodes[i] = std::move(finalized_node);
            ++i;

            node.reset();
        }
        return std::make_unique<iSaxFinalizedIndex<FTag>>(std::move(m_series_isax_prop), std::move(first_layer_symbols),
                                                          std::move(finalized_nodes), m_first_layer_num_bits,
                                                          m_alphabet_num_bits, *m_breakpoints);
    }

    const iSaxSplittableNode<T> *get_first_layer_node(const vec<iSaxWord> &isax_mins) const {
        MtsNumChannelsT num_channels = m_series_isax_prop->num_channels;
        SaxSegIndT num_seg_per_channel = m_series_isax_prop->num_seg_per_channel;

        vec<vec<SaxSymbolT>> symbols(num_channels, vec<SaxSymbolT>(num_seg_per_channel));
        for (MtsNumChannelsT c = 0; c < num_channels; ++c) {
            for (SaxSegIndT s = 0; s < num_seg_per_channel; ++s) symbols[c][s] = isax_mins[c][s];
        }
        auto node_it = m_first_layer.find(symbols);
        return node_it == m_first_layer.end() ? nullptr : node_it->second.get();
    }

   protected:
    std::unordered_map<vec<vec<SaxSymbolT>>, uptr<iSaxSplittableNode<T>>, SaxSymbolsHash> m_first_layer;
    SaxNumBitsT m_first_layer_num_bits, m_alphabet_num_bits;
    uptr<SeriesISaxProperties> m_series_isax_prop;
    size_t m_leaf_capacity;
    const vec<float> *m_breakpoints;
    uptr<IiSaxSplitStrategy<T>> m_split_strategy;

    virtual std::pair<uptr<iSaxFinalizedNode<FTag>>, vec<vec<SymbolType>>> finalize_first_layer_node(
        vec<vec<SaxSymbolT>> key_symbols, uptr<iSaxSplittableNode<T>> &node, iSaxWordSettings &isax_word_settings) = 0;

   private:
    void split_leaf(vec<iSaxWord> &isax_words, uptr<iSaxSplittableNode<T>> &node_ref) {
        auto *leaf = static_cast<iSaxSplittableLeaf<T> *>(node_ref.get());
        // Split the leaf
        auto [segment_ind, channel_ind] = m_split_strategy->get_split_ind(leaf, isax_words);
        SaxNumBitsT split_seg_bits = isax_words[channel_ind].get_num_bits()[segment_ind];
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
        auto symbol = isax_words[channel_ind][segment_ind];
        auto mid_breakpoint = m_breakpoints->at((symbol * 2 + 1) * (alphabet_size_ratio >> 1) - 1);

        // Distribute the mts_envelope across the two new leaves
        vec<SubsequenceInfo> left_subsequence_positions, right_subsequence_positions;
        vec<vec<T>> left_mts_summary, right_mts_summary;

        for (size_t i = 0; i < leaf->m_subsequence_positions.size(); ++i) {
            auto seg_min = leaf->m_summaries[i][channel_ind].get_isax_input();
            if (seg_min[segment_ind] <= mid_breakpoint) {
                left_subsequence_positions.push_back(leaf->m_subsequence_positions[i]);
                left_mts_summary.push_back(leaf->m_summaries[i]);
            } else {
                right_subsequence_positions.push_back(leaf->m_subsequence_positions[i]);
                right_mts_summary.push_back(leaf->m_summaries[i]);
            }
        }

        // Create new nodes
        size_t left_size = left_subsequence_positions.size(), right_size = right_subsequence_positions.size();
        auto new_internal = std::make_unique<iSaxSplittableInternal<T>>(SaxSplitIndex{segment_ind, channel_ind});
        new_internal->m_left =
            std::make_unique<iSaxSplittableLeaf<T>>(std::move(left_subsequence_positions), std::move(left_mts_summary));
        new_internal->m_right = std::make_unique<iSaxSplittableLeaf<T>>(std::move(right_subsequence_positions),
                                                                        std::move(right_mts_summary));

        // Replace the leaf with the new internal node
        node_ref = std::move(new_internal);

        // Check if further splitting is necessary
        bool split_left = left_size > m_leaf_capacity, split_right = right_size > m_leaf_capacity;
        if (split_left || split_right) {
            auto &parent = reinterpret_cast<uptr<iSaxSplittableInternal<T>> &>(node_ref);
            if (split_left) {
                uint8_t new_bit = 0;
                isax_words[channel_ind].append_to_symbol(segment_ind, new_bit);
                leaf = static_cast<iSaxSplittableLeaf<T> *>(parent->m_left.get());
                split_leaf(isax_words, parent->m_left);
                isax_words[channel_ind].remove_from_symbol(segment_ind);
            }
            if (split_right) {
                uint8_t new_bit = 1;
                isax_words[channel_ind].append_to_symbol(segment_ind, new_bit);
                leaf = static_cast<iSaxSplittableLeaf<T> *>(parent->m_right.get());
                split_leaf(isax_words, parent->m_right);
                isax_words[channel_ind].remove_from_symbol(segment_ind);
            }
        }
    }
};

// iSaxPaaIndex
class iSaxPaaIndex : public iSaxIndex<Paa> {
    using FTagPaa = typename iSaxIndexTraits<Paa>::FinalizedTag;
    using SymbolTypePaa = typename SaxTraits<FTagPaa>::SymbolType;

    std::pair<uptr<iSaxFinalizedNode<FTagPaa>>, vec<vec<SymbolTypePaa>>> finalize_first_layer_node(
        vec<vec<SaxSymbolT>> key_symbols, uptr<iSaxSplittableNode<Paa>> &node,
        iSaxWordSettings &isax_word_settings) override;

   public:
    iSaxPaaIndex(uptr<SeriesISaxProperties> series_isax_prop, SaxNumBitsT first_layer_num_bits, size_t leaf_capacity,
                 uptr<IiSaxSplitStrategy<Paa>> split_strategy);
};

// iSaxEnvelopeIndex
class iSaxEnvelopeIndex : public iSaxIndex<Envelope> {
    using FTagEnv = typename iSaxIndexTraits<Envelope>::FinalizedTag;
    using SymbolTypeEnv = typename SaxTraits<FTagEnv>::SymbolType;

    std::pair<uptr<iSaxFinalizedNode<FTagEnv>>, vec<vec<SymbolTypeEnv>>> finalize_first_layer_node(
        vec<vec<SaxSymbolT>> key_symbols, uptr<iSaxSplittableNode<Envelope>> &node,
        iSaxWordSettings &isax_word_settings) override;

   public:
    iSaxEnvelopeIndex(uptr<SeriesISaxProperties> series_isax_prop, SaxNumBitsT first_layer_num_bits,
                      size_t leaf_capacity, uptr<IiSaxSplitStrategy<Envelope>> split_strategy);
};

#endif  // ISAX_INDEX_HPP

#ifndef ISAX_INDEX_HPP
#define ISAX_INDEX_HPP

#include <type_traits>
#include <unordered_map>

#include "Enums/EntryInserterType.hpp"
#include "Index/Entry/IndexEntry.hpp"
#include "Index/FinalizedIndex.hpp"
#include "Index/Index.hpp"
#include "Index/Sax/SaxHelpers.hpp"
#include "Index/Segmentation/SegmentationStrategy/SegmentationStrategy.hpp"
#include "Index/Traits/IndexTraits.hpp"
#include "Index/iSaxIndex/FinalizedISaxIndex.hpp"
#include "Index/iSaxIndex/FinalizedISaxNode.hpp"
#include "Index/iSaxIndex/SplittableISaxNode.hpp"
#include "Index/iSaxIndex/iSaxSplitStrategy.hpp"
#include "Util/RunSettings/RunSettings.hpp"
#include "Util/Types/Pointers.hpp"

/**
 * @brief iSAX index
 * @tparam T Type data stored in the index
 */
template <typename T>
    requires DerivedFromEntryData<T>
class iSaxIndex : public IIndex<T>, public std::enable_shared_from_this<iSaxIndex<T>> {
    using FTag = typename IndexTraits<T>::FinalizedTag;
    using SymbolType = typename SaxTraits<FTag>::SymbolType;

    template <typename U>
        requires DerivedFromEntryData<U>
    friend class iSaxParallelInserter;

    bool m_merge_in_leaves;
    SaxNumBitsT m_first_layer_num_bits, m_alphabet_num_bits;
    size_t m_leaf_capacity;
    umap_hash<vec<vec<SaxSymbolT>>, uptr<SplittableISaxNode<T>>, SaxSymbolsHash> m_first_layer;
    const vec<Real> *m_breakpoints;
    sptr<ISegmentationStrategy> m_segmentation_strategy;
    uptr<IiSaxSplitStrategy<T>> m_split_strategy;
    iSaxWordFactory m_isax_word_factory;

    using m_first_layer_type = decltype(m_first_layer);

   public:
    /**
     * @brief Construct a new iSaxIndex object
     * @param first_layer_num_bits Number of bits used for symbols in the first layer
     * @param leaf_capacity Capacity of the leaf nodes
     * @param segmentation_strategy Segmentation strategy
     * @param split_strategy Split strategy
     * @param merge_in_leaves Whether to merge entries in the leaves
     */
    iSaxIndex(SaxNumBitsT first_layer_num_bits, size_t leaf_capacity, sptr<ISegmentationStrategy> segmentation_strategy,
              uptr<IiSaxSplitStrategy<T>> split_strategy, bool merge_in_leaves = false)
        : m_first_layer_num_bits(first_layer_num_bits),
          m_leaf_capacity(leaf_capacity),
          m_segmentation_strategy(segmentation_strategy),
          m_split_strategy(std::move(split_strategy)),
          m_merge_in_leaves(merge_in_leaves) {
        assert(first_layer_num_bits > 0);

        auto &RS = RunSettings::get_instance();
        m_alphabet_num_bits = RS.get_breakpoint_props().m_breakpoint_num_bits;
        m_breakpoints = &RS.get_breakpoints();

        m_isax_word_factory = [this](const vec<Real> &isax_input) {
            return iSaxWord(isax_input, *m_breakpoints, m_alphabet_num_bits,
                            vec<SaxNumBitsT>(isax_input.size(), m_first_layer_num_bits));
        };

        assert(m_breakpoints->size() == (1 << m_alphabet_num_bits) - 1);
        assert(m_alphabet_num_bits >= m_first_layer_num_bits);
    }

    iSaxIndex() = default;

    ~iSaxIndex() = default;

    void insert(IndexEntry<T> &entry) override {
        assert(entry.m_mts_summary.size() == static_cast<MtsNumChannelsT>(entry.m_mts_summary.size()));
        assert(entry.m_mts_summary[0].size() ==
               m_segmentation_strategy->get_num_segments(U(entry.m_mts_summary[0].size())));

        auto [symbols, isax_words] = get_entry_sax_symbols_and_isax(entry, m_isax_word_factory);

        auto node_it = m_first_layer.find(symbols);
        if (node_it == m_first_layer.end()) {
            insert_new_first_layer_node(symbols, entry);
        } else {
            insert_into_first_layer_node(isax_words, node_it, entry);
        }
    }

    void insert_entries(vec<IndexEntry<T>> &entries, EntryInserterType inserter_type) override;

    uptr<IFinalizedIndex<FTag>> finalize() override {
        size_t size_first_layer = m_first_layer.size();
        vec<vec<vec<SymbolType>>> first_layer_symbols(size_first_layer);
        vec<uptr<FinalizedISaxNode<FTag>>> finalized_nodes(size_first_layer);

        vec<size_t> cumulative_bucket_sizes(m_first_layer.bucket_count(), 0);
        for (size_t bucket = 1; bucket < m_first_layer.bucket_count(); ++bucket)
            cumulative_bucket_sizes[bucket] =
                cumulative_bucket_sizes[bucket - 1] + m_first_layer.bucket_size(bucket - 1);

        OMP_PRAGMA(omp parallel for)
        for (size_t bucket = 0; bucket < m_first_layer.bucket_count(); ++bucket) {
            size_t ind = cumulative_bucket_sizes[bucket];
            for (auto it = m_first_layer.begin(bucket); it != m_first_layer.end(bucket); ++it) {
                const auto &key_symbols = it->first;
                auto &node = it->second;

                auto [finalized_node, isax_symbols] = finalize_first_layer_node(key_symbols, node);
                first_layer_symbols[ind] = isax_symbols;
                finalized_nodes[ind] = std::move(finalized_node);
                ++ind;
                node.reset();
            }
        }

        auto &RS = RunSettings::get_instance();
        uint pos_per_env = RS.get_envelope_props().m_pos_per_env;
        return std::make_unique<FinalizedISaxIndex<FTag>>(m_segmentation_strategy, std::move(first_layer_symbols),
                                                          std::move(finalized_nodes), m_first_layer_num_bits,
                                                          m_alphabet_num_bits, *m_breakpoints, pos_per_env);
    }

    const SplittableISaxNode<T> *get_first_layer_node(const vec<iSaxWord> &isax_words) const {
        MtsNumChannelsT num_channels = static_cast<MtsNumChannelsT>(isax_words.size());
        SaxSegIndT num_seg_per_channel = static_cast<SaxSegIndT>(isax_words[0].size());

        vec<vec<SaxSymbolT>> symbols(num_channels, vec<SaxSymbolT>(num_seg_per_channel));
        for (MtsNumChannelsT c = 0; c < num_channels; ++c) {
            for (SaxSegIndT s = 0; s < num_seg_per_channel; ++s) symbols[c][s] = isax_words[c][s];
        }
        auto node_it = m_first_layer.find(symbols);
        return node_it == m_first_layer.end() ? nullptr : node_it->second.get();
    }

   private:
    std::pair<uptr<FinalizedISaxNode<FTag>>, vec<vec<SymbolType>>> finalize_first_layer_node(
        vec<vec<SaxSymbolT>> key_symbols, uptr<SplittableISaxNode<T>> &node) {
        MtsNumChannelsT num_channels = static_cast<MtsNumChannelsT>(key_symbols.size());
        SaxSegIndT num_seg_per_channel = static_cast<SaxSegIndT>(key_symbols[0].size());

        uptr<FinalizedISaxNode<FTag>> finalized_node;
        vec<vec<SymbolType>> symbols(num_channels, vec<SymbolType>(num_seg_per_channel));

        if constexpr (std::is_same_v<T, Paa>) {
            for (MtsNumChannelsT c = 0; c < num_channels; ++c) {
                for (SaxSegIndT s = 0; s < num_seg_per_channel; ++s) symbols[c][s] = SymbolType(key_symbols[c][s]);
            }
            finalized_node = get_paa_node_finalization_result(node, m_merge_in_leaves);
        } else {  // Envelope
            vec<iSaxWord> isax_max;
            std::tie(finalized_node, isax_max) = get_envelope_node_finalization_result(node, m_merge_in_leaves);
            SaxNumBitsT shift = m_alphabet_num_bits - m_first_layer_num_bits;
            for (MtsNumChannelsT c = 0; c < num_channels; ++c) {
                for (SaxSegIndT s = 0; s < num_seg_per_channel; ++s)
                    symbols[c][s] = SymbolType(key_symbols[c][s], isax_max[c].symbol_no_shift(s) >> shift);
            }
        }
        return {std::move(finalized_node), std::move(symbols)};
    }

    inline vec<iSaxWord> get_entry_isax(const IndexEntry<T> &entry) {
        return ::get_entry_isax(entry, m_isax_word_factory);
    }

    inline void calculate_first_layer_symbols(const IndexEntry<T> &entry, vec<vec<SaxSymbolT>> &symbols) {
        for (MtsNumChannelsT c = 0; c < symbols.size(); ++c) {
            auto isax_input = entry.m_mts_summary[c].get_isax_input();
            iSaxWord isax_word(isax_input, *m_breakpoints, m_alphabet_num_bits,
                               vec<SaxNumBitsT>(isax_input.size(), m_first_layer_num_bits));
            SaxSegIndT num_segments = static_cast<SaxSegIndT>(symbols[c].size());
            for (SaxSegIndT s = 0; s < num_segments; ++s) symbols[c][s] = isax_word[s];
        }
    }

    void insert_new_first_layer_node(const vec<vec<SaxSymbolT>> &symbols, IndexEntry<T> &entry) {
        m_first_layer.emplace(
            symbols, std::make_unique<iSaxSplittableLeaf<T>>(vec<SubsequenceInfo>{std::move(entry.m_subs_info)},
                                                             vec<vec<T>>{std::move(entry.m_mts_summary)}));
        auto &logger = IndexLogger::get_instance();
        logger.increment_count_col(ISC::NUM_NODES);
        logger.increment_count_col(ISC::NUM_LEAVES);
    }

    void insert_into_first_layer_node(vec<iSaxWord> &isax_words, m_first_layer_type::iterator &node_it,
                                      IndexEntry<T> &entry) {
        auto node = node_it->second.get();

        iSaxSplittableInternal<T> *parent = nullptr;
        uint8_t new_bit = 0;
        // Traverse tree until a leaf is reached
        while (!(node->is_leaf())) {
            parent = static_cast<iSaxSplittableInternal<T> *>(node);
            auto [segment_ind, channel_ind] = node->get_split_ind();
            new_bit = isax_words[channel_ind].apply_split(segment_ind);
            auto [left_child, right_child] = node->get_children();
            node = const_cast<SplittableISaxNode<T> *>(new_bit ? right_child : left_child);
        }
        // Reached a leaf => insert
        auto *leaf = static_cast<iSaxSplittableLeaf<T> *>(node);
        leaf->m_subsequence_infos.push_back(std::move(entry.m_subs_info));
        leaf->m_summaries.push_back(std::move(entry.m_mts_summary));

        // Split if needed (if the leaf size already surpassed the capacity before inserting the new entry, then a split
        // was attempted before and was unsuccessful => don't call split function again)
        if (leaf->m_subsequence_infos.size() == m_leaf_capacity + 1) {
            auto &node_ref = parent ? (new_bit ? parent->m_right : parent->m_left) : node_it->second;
            split_leaf(isax_words, node_ref);
        }
    }

    void split_leaf(vec<iSaxWord> &isax_words, uptr<SplittableISaxNode<T>> &node_ref) {
        auto *leaf = static_cast<iSaxSplittableLeaf<T> *>(node_ref.get());
        // Split the leaf
        auto [segment_ind, channel_ind] = m_split_strategy->get_split_ind(leaf, isax_words);

        std::optional<Real> mid_breakpoint = isax_words[channel_ind].get_mid_breakpoint(segment_ind, *m_breakpoints);
        if (!mid_breakpoint) return;

        auto &logger = IndexLogger::get_instance();
        logger.increment_count_col(ISC::NUM_NODES, 2);
        logger.increment_count_col(ISC::NUM_LEAVES);

        // Distribute the mts_envelope across the two new leaves
        vec<SubsequenceInfo> left_subsequence_positions, right_subsequence_positions;
        vec<vec<T>> left_mts_summary, right_mts_summary;

        for (size_t i = 0; i < leaf->m_subsequence_infos.size(); ++i) {
            auto seg_min = leaf->m_summaries[i][channel_ind].get_isax_input();
            if (seg_min[segment_ind] <= mid_breakpoint) {
                left_subsequence_positions.push_back(leaf->m_subsequence_infos[i]);
                left_mts_summary.push_back(leaf->m_summaries[i]);
            } else {
                right_subsequence_positions.push_back(leaf->m_subsequence_infos[i]);
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
                uint8_t old_bit = isax_words[channel_ind].apply_split(segment_ind);
                isax_words[channel_ind].set_new_bit(segment_ind, 0);
                leaf = static_cast<iSaxSplittableLeaf<T> *>(parent->m_left.get());
                split_leaf(isax_words, parent->m_left);
                isax_words[channel_ind].set_new_bit(segment_ind, old_bit);
                isax_words[channel_ind].unsplit(segment_ind);
            }
            if (split_right) {
                uint8_t old_bit = isax_words[channel_ind].apply_split(segment_ind);
                isax_words[channel_ind].set_new_bit(segment_ind, 1);
                leaf = static_cast<iSaxSplittableLeaf<T> *>(parent->m_right.get());
                split_leaf(isax_words, parent->m_right);
                isax_words[channel_ind].set_new_bit(segment_ind, old_bit);
                isax_words[channel_ind].unsplit(segment_ind);
            }
        }
    }

    void adapt_to_dataset(const vec<IndexEntry<T>> &dataset_entries) override {
        Real sum = 0, sum_sq = 0;
        uint count = 0;
        for (const auto &entry : dataset_entries) {
            for (const auto &summary : entry.m_mts_summary) {
                auto isax_input = summary.get_isax_input();
                for (auto val : isax_input) {
                    if (val == -INF || val == INF) continue;
                    sum += val;
                    sum_sq += val * val;
                    ++count;
                }
            }
        }
        auto [mu, sigma] = calculate_mu_and_sigma(sum, sum_sq, count);

        auto &RS = RunSettings::get_instance();
        RS.get_breakpoint_props().m_breakpoint_strategy->adapt_to_dataset(mu, sigma);
        RS.update_breakpoints();
    }
};

#endif  // ISAX_INDEX_HPP

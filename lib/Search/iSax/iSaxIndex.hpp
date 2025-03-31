#ifndef ISAX_INDEX_HPP
#define ISAX_INDEX_HPP

#include <unordered_map>
#include <type_traits>

#include "Util/typedefs.hpp"
#include "Util/constants.hpp"
#include "Util/utilities.hpp"
#include "Search/Index.hpp"
#include "Search/TopDownInserter.hpp"
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

// Forward declarations

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
        m_alphabet_num_bits = RS.get_isax_props().breakpoint_num_bits;
        m_breakpoints = &RS.get_breakpoints();

        assert(m_breakpoints->size() == (1 << m_alphabet_num_bits) - 1);
        assert(m_alphabet_num_bits >= m_first_layer_num_bits);
    }

    iSaxIndex() = default;

    ~iSaxIndex() = default;

    void insert(IndexEntry<T> &entry) override {
        MtsNumChannelsT num_channels = m_series_isax_prop->num_channels;
        SaxSegIndT num_seg_per_channel = m_series_isax_prop->num_seg_per_channel;

        assert(entry.mts_summary.size() == num_channels);
        assert(entry.mts_summary[0].size() == num_seg_per_channel);

        vec<iSaxWord> isax_words(num_channels);
        vec<vec<SaxSymbolT>> symbols(num_channels, vec<SaxSymbolT>(num_seg_per_channel));
        calculate_symbols_and_isax(entry, symbols, isax_words);

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
        vec<uptr<iSaxFinalizedNode<FTag>>> finalized_nodes(size_first_layer);

        iSaxWordSettings isax_word_settings = {
            vec<SaxNumBitsT>(m_series_isax_prop->num_seg_per_channel, m_alphabet_num_bits),
            m_alphabet_num_bits,
            *m_breakpoints,
        };

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

                auto [finalized_node, isax_symbols] = finalize_first_layer_node(key_symbols, node, isax_word_settings);
                first_layer_symbols[ind] = isax_symbols;
                finalized_nodes[ind] = std::move(finalized_node);
                ++ind;
                node.reset();
            }
        }
        return std::make_unique<iSaxFinalizedIndex<FTag>>(std::move(m_series_isax_prop), std::move(first_layer_symbols),
                                                          std::move(finalized_nodes), m_first_layer_num_bits,
                                                          m_alphabet_num_bits, *m_breakpoints);
    }

    const iSaxSplittableNode<T> *get_first_layer_node(const vec<iSaxWord> &isax_words) const {
        MtsNumChannelsT num_channels = m_series_isax_prop->num_channels;
        SaxSegIndT num_seg_per_channel = m_series_isax_prop->num_seg_per_channel;

        vec<vec<SaxSymbolT>> symbols(num_channels, vec<SaxSymbolT>(num_seg_per_channel));
        for (MtsNumChannelsT c = 0; c < num_channels; ++c) {
            for (SaxSegIndT s = 0; s < num_seg_per_channel; ++s) symbols[c][s] = isax_words[c][s];
        }
        auto node_it = m_first_layer.find(symbols);
        return node_it == m_first_layer.end() ? nullptr : node_it->second.get();
    }

   protected:
    virtual std::pair<uptr<iSaxFinalizedNode<FTag>>, vec<vec<SymbolType>>> finalize_first_layer_node(
        vec<vec<SaxSymbolT>> key_symbols, uptr<iSaxSplittableNode<T>> &node, iSaxWordSettings &isax_word_settings) = 0;

    umap_hash<vec<vec<SaxSymbolT>>, uptr<iSaxSplittableNode<T>>, SaxSymbolsHash> m_first_layer;
    SaxNumBitsT m_first_layer_num_bits, m_alphabet_num_bits;
    uptr<SeriesISaxProperties> m_series_isax_prop;
    size_t m_leaf_capacity;
    const vec<Real> *m_breakpoints;
    uptr<IiSaxSplitStrategy<T>> m_split_strategy;

    using m_first_layer_type = decltype(m_first_layer);

   private:
    void calculate_symbols_and_isax(const IndexEntry<T> &entry, vec<vec<SaxSymbolT>> &symbols,
                                    vec<iSaxWord> &isax_words) {
        for (MtsNumChannelsT c = 0; c < m_series_isax_prop->num_channels; ++c) {
            auto isax_input = entry.mts_summary[c].get_isax_input();
            isax_words[c] = iSaxWord(isax_input, {vec<SaxNumBitsT>(isax_input.size(), m_first_layer_num_bits),
                                                  m_alphabet_num_bits, *m_breakpoints});
            for (SaxSegIndT s = 0; s < m_series_isax_prop->num_seg_per_channel; ++s) symbols[c][s] = isax_words[c][s];
        }
    }

    void insert_new_first_layer_node(const vec<vec<SaxSymbolT>> &symbols, IndexEntry<T> &entry) {
        m_first_layer.emplace(
            symbols, std::make_unique<iSaxSplittableLeaf<T>>(vec<SubsequenceInfo>{std::move(entry.subsequence_info)},
                                                             vec<vec<T>>{std::move(entry.mts_summary)}));
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
            node = const_cast<iSaxSplittableNode<T> *>(new_bit ? right_child : left_child);
        }
        // Reached a leaf => insert
        auto *leaf = static_cast<iSaxSplittableLeaf<T> *>(node);
        leaf->m_subsequence_infos.push_back(std::move(entry.subsequence_info));
        leaf->m_summaries.push_back(std::move(entry.mts_summary));

        // Split if needed (if the leaf size already surpassed the capacity before inserting the new entry, then a split
        // was attempted before and was unsuccessful => don't call split function again)
        if (leaf->m_subsequence_infos.size() == m_leaf_capacity + 1) {
            auto &node_ref = parent ? (new_bit ? parent->m_right : parent->m_left) : node_it->second;
            split_leaf(isax_words, node_ref);
        }
    }

    void split_leaf(vec<iSaxWord> &isax_words, uptr<iSaxSplittableNode<T>> &node_ref) {
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
        auto &RS = RunSettings::get_instance();
        auto &isax_props = RS.get_isax_props();

        Real sum = 0, sum_sq = 0, count = 0;
        for (const auto &entry : dataset_entries) {
            for (const auto &summary : entry.mts_summary) {
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

        isax_props.breakpoint_strategy->adapt_to_dataset(mu, sigma);
        RS.update_breakpoints();
    }
};

// iSaxParallelInserter

template <typename T>
    requires DerivedFromEntryData<T>
class iSaxParallelInserter : public IEntryInserter<iSaxIndex<T>> {
   public:
    iSaxParallelInserter(sptr<iSaxIndex<T>> index) : m_index(index) {}

    void insert_entries(vec<IndexEntry<T>> &entries) override {
        umap_hash<vec<vec<SaxSymbolT>>, vec<uint>, SaxSymbolsHash> symbols_to_entry_inds;
        umap_hash<vec<vec<SaxSymbolT>>, vec<iSaxWord>, SaxSymbolsHash> symbols_to_isax_words;

        MtsNumChannelsT num_channels = m_index->m_series_isax_prop->num_channels;
        SaxSegIndT num_seg_per_channel = m_index->m_series_isax_prop->num_seg_per_channel;

        OMP_PRAGMA(omp parallel for)
        for (uint i = 0; i < entries.size(); ++i) {
            vec<iSaxWord> isax_words(num_channels);
            vec<vec<SaxSymbolT>> symbols(num_channels, vec<SaxSymbolT>(num_seg_per_channel));
            m_index->calculate_symbols_and_isax(entries[i], symbols, isax_words);

            OMP_PRAGMA(omp critical) {
                bool inserted = symbols_to_isax_words.try_emplace(symbols, std::move(isax_words)).second;
                if (inserted) {
                    m_index->insert_new_first_layer_node(symbols, entries[i]);
                } else {
                    symbols_to_entry_inds[symbols].push_back(i);
                }
            }
        }

        OMP_PRAGMA(omp parallel for)
        for (size_t bucket = 0; bucket < symbols_to_entry_inds.bucket_count(); ++bucket) {
            for (auto it = symbols_to_entry_inds.begin(bucket); it != symbols_to_entry_inds.end(bucket); ++it) {
                const auto &symbols = it->first;
                auto isax_words = symbols_to_isax_words.at(symbols);
                auto node_it = m_index->m_first_layer.find(symbols);
                for (uint ind : it->second) {
                    auto isax_words_copy = isax_words;
                    m_index->insert_into_first_layer_node(isax_words_copy, node_it, entries[ind]);
                }
            }
        }
    }

   private:
    sptr<iSaxIndex<T>> m_index;
};

template <typename T>
    requires DerivedFromEntryData<T>
void iSaxIndex<T>::insert_entries(vec<IndexEntry<T>> &entries, EntryInserterType inserter_type) {
    uptr<IEntryInserter<iSaxIndex<T>>> inserter;
    switch (inserter_type) {
        case TOP_DOWN:
            inserter = std::make_unique<TopDownInserter<iSaxIndex<T>>>(this->shared_from_this());
            break;
        case ISAX_PARALLEL:
            inserter = std::make_unique<iSaxParallelInserter<T>>(this->shared_from_this());
            break;
        default:
            throw std::invalid_argument("Invalid inserter type");
    }
    inserter->insert_entries(entries);
}

// iSaxPaaIndex
class iSaxPaaIndex : public iSaxIndex<Paa> {
    using FTagPaa = typename IndexTraits<Paa>::FinalizedTag;
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
    using FTagEnv = typename IndexTraits<Envelope>::FinalizedTag;
    using SymbolTypeEnv = typename SaxTraits<FTagEnv>::SymbolType;

    std::pair<uptr<iSaxFinalizedNode<FTagEnv>>, vec<vec<SymbolTypeEnv>>> finalize_first_layer_node(
        vec<vec<SaxSymbolT>> key_symbols, uptr<iSaxSplittableNode<Envelope>> &node,
        iSaxWordSettings &isax_word_settings) override;

   public:
    iSaxEnvelopeIndex(uptr<SeriesISaxProperties> series_isax_prop, SaxNumBitsT first_layer_num_bits,
                      size_t leaf_capacity, uptr<IiSaxSplitStrategy<Envelope>> split_strategy);
};

#endif  // ISAX_INDEX_HPP

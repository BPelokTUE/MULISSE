#ifndef TREE_ENVELOPE_INDEX_HPP
#define TREE_ENVELOPE_INDEX_HPP

#include <algorithm>
#ifndef DISABLE_PARALLELISM
#include <tbb/parallel_sort.h>
#endif
#include <queue>

#include "Util/typedefs.hpp"
#include "Util/Logging/QueryLogger.hpp"
#include "Search/Index.hpp"
#include "Search/Envelope/EnvelopeIndex.hpp"
#include "Search/Envelope/FlatEnvelopeIndex.hpp"
#include "Search/Envelope/EnvelopeGrouper.hpp"
#include "Search/Envelope/EnvelopeNode.hpp"
#include "Summarization/Envelope.hpp"
#include "Summarization/FinalizedTraits.hpp"
#include "Summarization/InvSax.hpp"

class FinalizedTreeEnvelopeIndex : public FinalizedEnvelopeIndex {
   public:
    FinalizedTreeEnvelopeIndex() = default;

    /**
     * @brief Construct a new FinalizedTreeEnvelopeIndex instance
     * @param segmentation_strategy The segmentation strategy to use
     * @param pos_per_env The number of positions per envelope
     * @param nodes The envelope nodes in the first layer of the tree
     */
    FinalizedTreeEnvelopeIndex(sptr<ISegmentationStrategy> segmentation_strategy, const uint pos_per_env,
                               vec<uptr<EnvelopeNode>> &&nodes)
        : FinalizedEnvelopeIndex(segmentation_strategy, pos_per_env), m_first_layer_nodes(std::move(nodes)) {}

    /**
     * @brief Get the first layer nodes of the tree
     * @return The first layer nodes of the tree
     */
    const vec<const EnvelopeNode *> get_first_layer_nodes() const {
        vec<const EnvelopeNode *> nodes(m_first_layer_nodes.size());
        for (size_t i = 0; i < nodes.size(); ++i) nodes[i] = m_first_layer_nodes[i].get();
        return nodes;
    }

   private:
    vec<uptr<EnvelopeNode>> m_first_layer_nodes;

    MAKE_SERIALIZABLE((m_segmentation_strategy, m_pos_per_env, m_first_layer_nodes));
};

class TreeEnvelopeIndex : public EnvelopeIndex, public std::enable_shared_from_this<TreeEnvelopeIndex> {
   public:
    /**
     * @brief Construct a new TreeEnvelopeIndex instance
     * @param segmentation_strategy The segmentation strategy to use
     * @param pos_per_env Number of positions per envelope
     * @param grouper The EnvelopeGrouper to use for grouping envelope entries
     */
    TreeEnvelopeIndex(sptr<ISegmentationStrategy> segmentation_strategy, const uint pos_per_env,
                      uptr<IEnvelopeGrouper> grouper)
        : EnvelopeIndex(segmentation_strategy, pos_per_env), m_grouper(std::move(grouper)) {}

    void insert_entries(vec<IndexEntry<Envelope>> &entries, EntryInserterType inserter_type) override {
        uptr<IEntryInserter<TreeEnvelopeIndex>> inserter;
        switch (inserter_type) {
            case ISAX_PARALLEL:  // Temporary solution to support two-stage indexes
            case TOP_DOWN:
                inserter = std::make_unique<TopDownInserter<TreeEnvelopeIndex>>(this->shared_from_this());
                break;
            default:
                throw std::invalid_argument("Invalid inserter type");
        }
        inserter->insert_entries(entries);
    };

    uptr<IFinalizedIndex<EnvelopeTag>> finalize() override {
        // 1. Group entries
        m_first_layer_nodes = m_grouper->group_envelope_entries(m_entries);

        // 2. Merge entries in leaves of the tree

        // 3. Return the finalized index (this)
        return std::make_unique<FinalizedTreeEnvelopeIndex>(m_segmentation_strategy, m_pos_per_env,
                                                            std::move(m_first_layer_nodes));
    }

   private:
    uptr<IEnvelopeGrouper> m_grouper;
    vec<uptr<EnvelopeNode>> m_first_layer_nodes;
};

/** @brief Priority queue entry for searching with TreeEnvelopeIndexSearch */
struct PQueueEnvelopeNodeEntry {
    Real m_min_dist_squared;
    const EnvelopeNode *m_node;

    bool operator<(const PQueueEnvelopeNodeEntry &other) const { return m_min_dist_squared > other.m_min_dist_squared; }
};

template <SearchType S, DistanceType D, bool QS = false>
class TreeEnvelopeIndexSearch : public EnvelopeIndexSearch<S, D, QS> {
   public:
    TreeEnvelopeIndexSearch(uptr<FinalizedTreeEnvelopeIndex> index) : m_index(std::move(index)) {}

    SearchResults search(const vec<vec<Real>> &query, const SearchOptions &opts, ResultSet<S> &result_set,
                         const DistanceMeasure<S, D, QS> &distance_measure, std::ifstream &dataset_ifs,
                         const vec<uint> *real_query_inds) const override {
        auto &logger = QueryLogger::get_instance();

        uint series_len = RunSettings::get_instance().get_dataset_props().m_series_len;
        auto segmentation_strategy = m_index->get_segmentation_strategy();
        auto [query_paa, query_len] = this->get_query_paa_and_len(query, segmentation_strategy, real_query_inds);

        std::priority_queue<PQueueEnvelopeNodeEntry> pq;

        logger.start_timer(QC::FIRST_LAYER_TIME_S);
        for (auto &node : m_index->get_first_layer_nodes()) {
            Real min_dist_squared =
                this->get_min_dist_squared(node->get_envelopes(), query_paa, distance_measure, segmentation_strategy);
            pq.push({min_dist_squared, node});
        }
        logger.stop_timer(QC::FIRST_LAYER_TIME_S);

        logger.increment_count_col(QC::NUM_MIN_DIST_CALCULATED, U(pq.size()));

        while (!pq.empty()) {
            auto entry = pq.top();
            pq.pop();

            Real lb = result_set.get_distance_lb();
            if (entry.m_min_dist_squared >= lb) break;

            if (!entry.m_node->is_leaf()) {
                for (auto &child : entry.m_node->get_children()) {
                    Real min_dist_squared = this->get_min_dist_squared(child->get_envelopes(), query_paa,
                                                                       distance_measure, segmentation_strategy);
                    logger.increment_count_col(QC::NUM_MIN_DIST_CALCULATED);
                    if (min_dist_squared < lb) pq.push({min_dist_squared, child});
                }
            } else {
                for (auto subs_info : *(entry.m_node->get_subsequence_infos())) {
                    if (this->skip_entry(query_len, series_len, subs_info)) continue;
                    this->update_result_set(subs_info, m_index->get_pos_per_env(), query, query_len, result_set,
                                            distance_measure, dataset_ifs, real_query_inds);
                }
            }
        }
        return {result_set.get_results(), true};
    }

   private:
    uptr<FinalizedTreeEnvelopeIndex> m_index;
};

#endif  // ENVELOPE_INDEX_HPP

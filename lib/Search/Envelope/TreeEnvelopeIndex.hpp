#ifndef TREE_ENVELOPE_INDEX_HPP
#define TREE_ENVELOPE_INDEX_HPP

#include <algorithm>
#ifndef DISABLE_PARALLELISM
#include <tbb/parallel_sort.h>
#endif
#include <queue>

#include "Search/Index.hpp"
#include "Search/Envelope/FlatEnvelopeIndex.hpp"
#include "Search/Envelope/EnvelopeGrouper.hpp"
#include "Search/Envelope/EnvelopeNode.hpp"
#include "Summarization/Envelope.hpp"
#include "Summarization/InvSax.hpp"

class FinalizedTreeEnvelopeIndex : public FinalizedFlatEnvelopeIndex {
   public:
    /**
     * @brief Construct a new FinalizedTreeEnvelopeIndex instance
     * @param nodes The envelope nodes in the first layer of the tree
     */
    FinalizedTreeEnvelopeIndex(vec<uptr<EnvelopeNode>> nodes) : m_nodes(std::move(nodes)) {}

   private:
    vec<uptr<EnvelopeNode>> m_nodes;
};

class TreeEnvelopeIndex : public FlatEnvelopeIndex {
   public:
    /**
     * @brief Construct a new TreeEnvelopeIndex instance
     * @param grouper The EnvelopeGrouper to use for grouping envelope entries
     */
    TreeEnvelopeIndex(uptr<IEnvelopeGrouper> grouper) : m_grouper(std::move(grouper)) {}

    uptr<IFinalizedIndex<EnvelopeTag>> finalize() override {
        // 1. Group entries
        m_first_layer_nodes = m_grouper->group_envelope_entries(m_entries);

        // 2. Merge entries in leaves of the tree

        // 3. Return the finalized index (this)
        return std::make_unique<FinalizedTreeEnvelopeIndex>(std::move(m_first_layer_nodes));
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

template <typename S, typename D, bool QS = false>
class TreeEnvelopeIndexSearch : public EnvelopeIndexSearch<S, D, QS> {
   public:
    SearchResults search(const vec<vec<Real>> &query, const SearchOptions &opts, ResultSet<S> &result_set,
                         const DistanceMeasure<S, D, QS> &distance_measure, std::ifstream &dataset_ifs,
                         const vec<uint> *real_query_inds) const override {
        auto [query_paa, query_len] = this->get_query_paa_and_len(query, m_index->get_segment_len(), real_query_inds);

        std::priority_queue<PQueueEnvelopeNodeEntry> pq;
        for (auto &node : m_index->m_nodes) {
            Real min_dist_squared = get_min_dist_squared(node->get_envelopes(), query_paa, distance_measure);
            pq.push({min_dist_squared, node.get()});
        }

        while (!pq.empty()) {
            auto entry = pq.top();
            pq.pop();

            Real lb = result_set.get_distance_lb();
            if (entry.m_min_dist_squared >= lb) break;

            if (!entry.m_node->is_leaf()) {
                for (auto &child : entry.m_node->get_children()) {
                    Real min_dist_squared = get_min_dist_squared(child->get_envelopes(), query_paa, distance_measure);
                    if (min_dist_squared < lb) pq.push({min_dist_squared, child.get()});
                }
            } else {
                for (auto subs_info : *(entry.m_node->get_subsequence_infos())) {
                    if (this->skip_entry(query_len, m_index->get_segment_len(), subs_info)) continue;
                    update_result_set(subs_info, query, result_set, distance_measure, dataset_ifs, real_query_inds);
                }
            }
        }
        return result_set.get_results();
    }

   private:
    uptr<FinalizedTreeEnvelopeIndex> m_index;
};

#endif  // ENVELOPE_INDEX_HPP

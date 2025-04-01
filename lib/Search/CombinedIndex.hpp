#ifndef COMBINED_INDEX_HPP
#define COMBINED_INDEX_HPP

#include "Search/Index.hpp"
#include "Search/Envelope/EnvelopeIndex.hpp"
#include "Search/iSax/iSaxFinalizedIndex.hpp"

template <typename FTag>
    requires ValidEntryTraitsTag<FTag>
class CombinedFinalizedIndex : public IFinalizedIndex<FTag> {
   public:
    CombinedFinalizedIndex() = default;

    CombinedFinalizedIndex(vec<sptr<IFinalizedIndex<FTag>>> approx_indexes, sptr<IFinalizedIndex<FTag>> exact_index)
        : m_approx_indexes(std::move(approx_indexes)), m_exact_index(std::move(exact_index)) {}

    vec<sptr<IFinalizedIndex<FTag>>> &get_approx_indexes() { return m_approx_indexes; }

    sptr<IFinalizedIndex<FTag>> &get_exact_index() { return m_exact_index; }

   private:
    vec<sptr<IFinalizedIndex<FTag>>> m_approx_indexes;
    sptr<IFinalizedIndex<FTag>> m_exact_index;

    MAKE_SERIALIZABLE((m_approx_indexes, m_exact_index));
};

template <typename T>
    requires DerivedFromEntryData<T>
class CombinedIndex : public IIndex<T> {
    using FTag = typename IndexTraits<T>::FinalizedTag;

   public:
    CombinedIndex() = default;

    CombinedIndex(vec<sptr<IIndex<T>>> approx_indexes, sptr<IIndex<T>> exact_index)
        : m_approx_indexes(std::move(approx_indexes)), m_exact_index(std::move(exact_index)) {}

    uptr<IFinalizedIndex<FTag>> finalize() override {
        vec<sptr<IFinalizedIndex<FTag>>> approx_indexes;
        for (auto &index : m_approx_indexes) approx_indexes.push_back(index->finalize());
        sptr<IFinalizedIndex<FTag>> exact_index = m_exact_index->finalize();

        return uptr<IFinalizedIndex<FTag>>(new CombinedFinalizedIndex<FTag>(approx_indexes, exact_index));
    }

    void insert_entries(vec<IndexEntry<T>> &entries, EntryInserterType inserter_type) override {
        for (auto &index : m_approx_indexes) {
            // Only move the entries when doing the last insertions
            auto entries_copy = entries;
            index->insert_entries(entries_copy, inserter_type);
        }
        m_exact_index->insert_entries(entries, inserter_type);
    }

    void insert(IndexEntry<T> &entry) override {
        for (auto &index : m_approx_indexes) {
            // Only move the entry when doing the last insertions
            auto entry_copy = entry;
            index->insert(entry_copy);
        }
        m_exact_index->insert(entry);
    }

   private:
    vec<sptr<IIndex<T>>> m_approx_indexes;
    sptr<IIndex<T>> m_exact_index;
};

template <SearchType S, DistanceType D, bool QS = false>
class CombinedSearch : public ISearchMethod<S, D, QS> {
   public:
    CombinedSearch(vec<sptr<ISearchMethod<S, D, QS>>> approx_methods, sptr<ISearchMethod<S, D, QS>> exact_method)
        : m_approx_methods(std::move(approx_methods)), m_exact_method(std::move(exact_method)) {}

    SearchResults search(const vec<vec<Real>> &query, const SearchOptions &opts, ResultSet<S> &result_set,
                         const DistanceMeasure<S, D, QS> &distance_measure, std::ifstream &dataset_ifs,
                         const vec<uint> *real_query_inds) const override {
        auto opts_approx = opts;
        opts_approx.exact = false;
        for (auto &method : m_approx_methods) {
            auto search_results =
                method->search(query, opts_approx, result_set, distance_measure, dataset_ifs, real_query_inds);
            if (search_results.exact) return search_results;
        }
        return m_exact_method->search(query, opts, result_set, distance_measure, dataset_ifs, real_query_inds);
    }

   private:
    vec<sptr<ISearchMethod<S, D, QS>>> m_approx_methods;
    sptr<ISearchMethod<S, D, QS>> m_exact_method;
};

#endif  // COMBINED_INDEX_HPP

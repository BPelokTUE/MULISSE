#ifndef CHAIN_INDEX_HPP
#define CHAIN_INDEX_HPP

#include "Util/typedefs.hpp"
#include "Summarization/IndexEntry.hpp"
#include "Search/Index.hpp"

/**
 * @brief Set of finalized indexes, intended to be used in a chain, with the approximate indexes being used first and
 * the exact index at the end.
 * @tparam FTag The traits of the entries in the index
 */
template <typename FTag>
    requires ValidEntryTraitsTag<FTag>
class ChainFinalizedIndex : public IFinalizedIndex<FTag> {
   public:
    ChainFinalizedIndex() = default;

    ChainFinalizedIndex(vec<uptr<IFinalizedIndex<FTag>>> approx_indexes, uptr<IFinalizedIndex<FTag>> exact_index)
        : m_approx_indexes(std::move(approx_indexes)), m_exact_index(std::move(exact_index)) {}

    void save(const str &out_file, ArchiveType ar_type) override {
        for (uint approx_ind = 0; approx_ind < m_approx_indexes.size(); ++approx_ind)
            m_approx_indexes[approx_ind]->save(get_index_file_path(out_file, false, ar_type, approx_ind), ar_type);
        m_exact_index->save(get_index_file_path(out_file, true, ar_type), ar_type);
    }

    void load(const str &in_file, ArchiveType ar_type) override {
        for (uint approx_ind = 0; approx_ind < m_approx_indexes.size(); ++approx_ind)
            m_approx_indexes[approx_ind]->load(get_index_file_path(in_file, false, ar_type, approx_ind), ar_type);
        m_exact_index->load(get_index_file_path(in_file, true, ar_type), ar_type);
    }

    size_t get_size_on_disk(const str &index_file, const ArchiveType ar_type) const override {
        size_t size = 0;
        for (uint approx_ind = 0; approx_ind < m_approx_indexes.size(); ++approx_ind)
            size += m_approx_indexes[approx_ind]->get_size_on_disk(
                get_index_file_path(index_file, false, ar_type, approx_ind));
        size += m_exact_index->get_size_on_disk(get_index_file_path(index_file, true, ar_type));
        return size;
    }

    IFinalizedIndex<FTag> *release_approx_index(uint index) {
        assert(index < m_approx_indexes.size());
        return m_approx_indexes[index].release();
    }

    IFinalizedIndex<FTag> *release_exact_index() { return m_exact_index.release(); }

   private:
    vec<uptr<IFinalizedIndex<FTag>>> m_approx_indexes;
    uptr<IFinalizedIndex<FTag>> m_exact_index;

    /**
     * @brief Get the file path for an index file
     * @param path_base The base path of the file
     * @param exact Whether the index is exact or approximate
     * @param ar_type The file format of the index
     * @param approx_ind The index of the approximate index, if applicable
     * @return The file path for the index
     */
    str get_index_file_path(const str &path_base, bool exact, ArchiveType ar_type, uint approx_ind = 0) const {
        auto [base, extension] = get_file_base_and_extension(path_base);
        extension = extension.empty() ? get_archive_extension(ar_type) : extension;
        return base + (exact ? "_exact" : "_approx_" + std::to_string(approx_ind)) + extension;
    }
};

/**
 * @brief Set of indexes intended to be used in a chain, with the approximate indexes being used first and the exact
 * index at the end
 * @tparam T The type of the entries in the index
 */
template <typename T>
    requires DerivedFromEntryData<T>
class ChainIndex : public IIndex<T> {
    using FTag = typename IndexTraits<T>::FinalizedTag;

   public:
    ChainIndex() = default;

    ChainIndex(vec<sptr<IIndex<T>>> approx_indexes, sptr<IIndex<T>> exact_index)
        : m_approx_indexes(std::move(approx_indexes)), m_exact_index(std::move(exact_index)) {}

    uptr<IFinalizedIndex<FTag>> finalize() override {
        vec<uptr<IFinalizedIndex<FTag>>> approx_indexes;
        for (auto &index : m_approx_indexes) approx_indexes.push_back(index->finalize());
        uptr<IFinalizedIndex<FTag>> exact_index = m_exact_index->finalize();

        return uptr<IFinalizedIndex<FTag>>(
            new ChainFinalizedIndex<FTag>(std::move(approx_indexes), std::move(exact_index)));
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

/**
 * @brief Set of search methods intended to be used in a chain, with the approximate methods being used first and the
 * exact method at the end. If the exact answer is found anywhere in the chain, the search stops.
 * @tparam S The type of the search to execute
 * @tparam D The type of the distance to use
 * @tparam QS Whether the query data points are sorted
 */
template <SearchType S, DistanceType D, bool QS = false>
class ChainSearch : public ISearchMethod<S, D, QS> {
   public:
    ChainSearch(vec<uptr<ISearchMethod<S, D, QS>>> approx_methods, uptr<ISearchMethod<S, D, QS>> exact_method)
        : m_approx_methods(std::move(approx_methods)), m_exact_method(std::move(exact_method)) {}

    SearchResults search(const vec<vec<Real>> &query, const SearchOptions &opts, ResultSet<S> &result_set,
                         const DistanceMeasure<S, D, QS> &distance_measure, std::ifstream &dataset_ifs,
                         const vec<uint> *real_query_inds) const override {
        auto opts_approx = opts;
        opts_approx.m_exact = false;
        for (auto &method : m_approx_methods) {
            auto search_results =
                method->search(query, opts_approx, result_set, distance_measure, dataset_ifs, real_query_inds);
            if (search_results.m_exact) return search_results;
        }
        return m_exact_method->search(query, opts, result_set, distance_measure, dataset_ifs, real_query_inds);
    }

   private:
    vec<uptr<ISearchMethod<S, D, QS>>> m_approx_methods;
    uptr<ISearchMethod<S, D, QS>> m_exact_method;
};

#endif  // CHAIN_INDEX_HPP

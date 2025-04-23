#ifndef LENGTH_GROUPING_INDEX_HPP
#define LENGTH_GROUPING_INDEX_HPP

#include <filesystem>

#include "Util/typedefs.hpp"
#include "Search/Index.hpp"
#include "Search/Options/IndexOptions.hpp"
#include "Summarization/IndexEntry.hpp"

namespace fs = std::filesystem;

/**
 * @brief Group of finalized indexes each containing entries summarizing data about subsequences in different length
 * ranges.
 * @tparam FTag The traits of the entries in the index
 */
template <typename FTag>
    requires ValidEntryTraitsTag<FTag>
class LengthGroupingFinalizedIndex : public IFinalizedIndex<FTag> {
   public:
    /**
     * @brief Construct a new LengthGroupingFinalizedIndex object
     * @param indexes The indexes to use for each length group
     * @param l_min Minimum query length
     * @param l_max Maximum query length
     */
    LengthGroupingFinalizedIndex(vec<uptr<IFinalizedIndex<FTag>>> indexes, uint l_min, uint l_max)
        : m_indexes(std::move(indexes)), m_l_min(l_min), m_l_max(l_max) {
        assert(l_min > 0);
        assert(l_max > 0);
        assert(l_min <= l_max);
    }

    void save(const str &out_file, ArchiveType ar_type) override {
        str base = get_file_base_and_extension(out_file).first;

        if (!fs::exists(base)) fs::create_directories(base);

        for (uint l_ind = 0; l_ind < m_indexes.size(); ++l_ind)
            m_indexes[l_ind]->save(fs::path(base) / get_index_file_name(l_ind), ar_type);
    }

    void load(const str &in_file, ArchiveType ar_type) override {
        str base = get_file_base_and_extension(in_file).first;

        for (uint l_ind = 0; l_ind < m_indexes.size(); ++l_ind)
            m_indexes[l_ind]->load(fs::path(base) / get_index_file_name(l_ind), ar_type);
    }

    size_t get_size_on_disk(const str &index_file, const ArchiveType ar_type) const override {
        str base = get_file_base_and_extension(index_file).first;

        size_t size = 0;
        for (uint l_ind = 0; l_ind < m_indexes.size(); ++l_ind)
            size += m_indexes[l_ind]->get_size_on_disk(fs::path(base) / get_index_file_name(l_ind), ar_type);
        return size;
    }

    IFinalizedIndex<FTag> *release_index(uint length_group) {
        assert(length_group < m_indexes.size());
        return m_indexes[length_group].release();
    }

   private:
    vec<uptr<IFinalizedIndex<FTag>>> m_indexes;
    uint m_l_min, m_l_max;

    str get_index_file_name(uint length_group) const { return "LG_" + std::to_string(length_group); }
};

/**
 * @brief Group of indexes each containing entries summarizing data about subsequences in different length ranges.
 * @tparam T The type of the entries in the index
 */
template <typename T>
    requires DerivedFromEntryData<T>
class LengthGroupingIndex : public IIndex<T> {
   public:
    using FTag = typename IndexTraits<T>::FinalizedTag;

    /**
     * @brief Construct a new LengthGroupingIndex object
     * @param indexes The indexes to use for each length group
     * @param l_min Minimum query length
     * @param l_max Maximum query length
     */
    LengthGroupingIndex(vec<sptr<IIndex<T>>> indexes, uint l_min, uint l_max)
        : m_indexes(indexes), m_l_min(l_min), m_l_max(l_max) {
        assert(l_min > 0);
        assert(l_max > 0);
        assert(l_min <= l_max);
    }

    void insert_entry_groups(vec<vec<IndexEntry<T>>> &entry_groups, EntryInserterType inserter_type) override {
        OMP_PRAGMA(omp parallel for)
        for (uint l_ind = 0; l_ind < U(entry_groups.size()); ++l_ind) {
            auto &entry_group = entry_groups[l_ind];
            m_indexes[l_ind]->insert_entries(entry_group, inserter_type);
        }
    }

    void insert_entries(vec<IndexEntry<T>> &entries, EntryInserterType inserter_type) override {
        for (auto &entry : entries) {
            m_indexes[get_entry_length_group(entry)]->insert(entry);
        }
    }

    void insert(IndexEntry<T> &entry) override { m_indexes[get_entry_length_group(entry)]->insert(entry); }

    uptr<IFinalizedIndex<FTag>> finalize() override {
        vec<uptr<IFinalizedIndex<FTag>>> finalized_indexes(m_indexes.size());
        for (uint l_ind = 0; l_ind < m_indexes.size(); ++l_ind) {
            finalized_indexes[l_ind] = m_indexes[l_ind]->finalize();
        }
        return uptr<IFinalizedIndex<FTag>>(
            new LengthGroupingFinalizedIndex<FTag>(std::move(finalized_indexes), m_l_min, m_l_max));
    }

    void adapt_to_dataset_groups(const vec<vec<IndexEntry<T>>> &dataset_entry_groups) override {
        for (uint l_ind = 0; l_ind < dataset_entry_groups.size(); ++l_ind) {
            m_indexes[l_ind]->adapt_to_dataset(dataset_entry_groups[l_ind]);
        }
    }

   private:
    vec<sptr<IIndex<T>>> m_indexes;
    uint m_l_min, m_l_max;

    inline uint get_entry_length_group(const IndexEntry<T> &entry) const {
        return RunSettings::get_instance().get_length_group(entry.m_subs_info.m_length);
    }
};

/**
 * @brief Search method for using length grouped indexes
 * @tparam S The type of the search to execute
 * @tparam D The type of the distance to use
 * @tparam QS Whether the query data points are sorted
 */
template <SearchType S, DistanceType D, bool QS = false>
class LengthGroupingIndexSearch : public ISearchMethod<S, D, QS> {
   public:
    /**
     * @brief Construct a new LengthGroupingIndexSearch object
     * @param search_methods The search methods to use for each length group
     * @param l_min Minimum query length
     * @param l_max Maximum query length
     */
    LengthGroupingIndexSearch(vec<uptr<ISearchMethod<S, D, QS>>> search_methods, uint l_min, uint l_max)
        : m_search_methods(std::move(search_methods)), m_l_min(l_min), m_l_max(l_max) {
        assert(l_min > 0);
        assert(l_max > 0);
        assert(l_min <= l_max);
    }

    SearchResults search(const vec<vec<Real>> &query, const SearchOptions &opts, ResultSet<S> &result_set,
                         const DistanceMeasure<S, D, QS> &distance_measure, std::ifstream &dataset_ifs,
                         const vec<uint> *real_query_inds) const override {
        uint query_len;
        for (auto &channel : query) {
            if (!channel.empty()) {
                query_len = U(channel.size());
                break;
            }
        }
        uint length_group = RunSettings::get_instance().get_length_group(query_len);
        return m_search_methods[length_group]->search(query, opts, result_set, distance_measure, dataset_ifs,
                                                      real_query_inds);
    }

   private:
    vec<uptr<ISearchMethod<S, D, QS>>> m_search_methods;
    uint m_l_min, m_l_max;
};

#endif  // LENGTH_GROUPING_INDEX_HPP

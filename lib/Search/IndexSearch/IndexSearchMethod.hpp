#ifndef SEARCH_INDEXSEARCH_INDEXSEARCHMETHOD_HPP
#define SEARCH_INDEXSEARCH_INDEXSEARCHMETHOD_HPP

#include <unordered_set>

#include "Enums/DistanceType.hpp"
#include "Enums/SearchType.hpp"
#include "Index/Traits/EntryTags.hpp"
#include "Search/SearchMethod.hpp"
#include "Util/Types/SubsequenceInfo.hpp"

template <typename FTag, bool EW = false>
struct ExtraMembersForIndexSearchMethod {};

template <>
struct ExtraMembersForIndexSearchMethod<PaaTag, false> {
    std::unordered_set<SubsequencePosition, SubsequencePositionHash> m_examined_positions;
};

template <typename FTag>
struct ExtraMembersForIndexSearchMethod<FTag, true> {
    std::unordered_set<uint> m_examined_series;
};

/**
 * @brief Abstract class for index-based search methods
 * @tparam FTag The traits of the entries in the index
 * @tparam S SearchType to execute
 * @tparam D DistanceType to use
 * @tparam EW Whether to examine the whole series when a subsequence examination is performed
 * @tparam SQ Whether the query is sorted or not
 */
template <typename FTag, SearchType S, DistanceType D, bool EW = false, bool SQ = false>
    requires ValidEntryTraitsTag<FTag>
class IndexSearchMethod : public ISearchMethod<S, D, SQ> {
   public:
    inline void reset() override {
        if constexpr (EW) {
            m_extras.m_examined_series.clear();
        } else if constexpr (std::is_same_v<FTag, PaaTag>) {
            m_extras.m_examined_positions.clear();
        }
    }

   protected:
    /**
     * @brief Check if the given time series can be skipped during search
     * @param series_ind Index of the time series in the dataset
     * @return `true` if the time series can be skipped, `false` otherwise
     */
    inline bool skip_series(uint series_ind) {
        if constexpr (EW) {
            if (m_extras.m_examined_series.contains(series_ind)) return true;
            m_extras.m_examined_series.insert(series_ind);
        }
        return false;
    }

    /**
     * @brief Check if the given starting position can be ignored/skipped during search
     * @param subs_position Position of the subsequence in the dataset
     * @return `true` if the position can be skipped, `false` otherwise
     */
    inline bool skip_position(const SubsequencePosition &subs_position) {
        if constexpr (!EW && std::is_same_v<FTag, PaaTag>) {
            if (m_extras.m_examined_positions.contains(subs_position)) return true;
            m_extras.m_examined_positions.insert(subs_position);
        }
        return false;
    }

    /**
     * @brief Check if the given entry can be ignored/skipped during search
     * @param query_len Length of the query
     * @param series_len Length of the series
     * @param subs_info Information about the subsequence in the dataset
     * @return `true` if the entry can be skipped, `false` otherwise
     */
    inline bool skip_entry(const uint query_len, const uint series_len, const SubsequenceInfo &subs_info) {
        if constexpr (std::is_same_v<FTag, PaaTag>) {
            return subs_info.m_length < query_len;
        } else if constexpr (std::is_same_v<FTag, EnvelopeTag>) {
            return series_len - subs_info.m_position.m_start < query_len;
        }
        return false;
    }

    inline vec<vec<Real>> read_data(const SubsequenceInfo &subs_info, const vec<vec<Real>> &query,
                                    std::ifstream &dataset_ifs, size_t data_to_read, uint series_len) {
        MtsNumChannelsT num_channels = static_cast<MtsNumChannelsT>(query.size());
        vec<vec<Real>> data(num_channels);

        for (MtsNumChannelsT c = 0; c < num_channels; ++c) {
            if (query[c].empty()) continue;

            data[c].resize(data_to_read);
            if constexpr (EW) {
                auto series_start_info = subs_info;
                series_start_info.m_position.m_start = 0;
                dataset_ifs.seekg(series_start_info.get_file_pos(series_len, num_channels, c));
            } else {
                dataset_ifs.seekg(subs_info.get_file_pos(series_len, num_channels, c));
            }
            dataset_ifs.read(reinterpret_cast<char *>(data[c].data()),
                             static_cast<std::streamsize>(data_to_read * sizeof(Real)));
        }
        return data;
    }

   private:
    ExtraMembersForIndexSearchMethod<FTag, EW> m_extras;
};

#endif  // SEARCH_INDEXSEARCH_INDEXSEARCHMETHOD_HPP

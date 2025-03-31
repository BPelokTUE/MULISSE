#ifndef SEQUENTIAL_SCAN_HPP
#define SEQUENTIAL_SCAN_HPP

#include <fstream>

#include "Util/typedefs.hpp"
#include "Util/RunSettings.hpp"
#include "Util/Logger.hpp"
#include "Search/SearchMethod.hpp"
#include "Search/ResultSet.hpp"
#include "Search/Options/SearchOptions.hpp"

/**
 * @brief Class for sequential scan search method
 * @tparam S SearchType to execute
 * @tparam D DistanceType to use
 * @tparam QS Whether to sort the query or not
 */
template <SearchType S, DistanceType D, bool QS = false>
class SequentialScan : public ISearchMethod<S, D, QS> {
   public:
    SequentialScan() = default;

    vec<SearchResult> search(const vec<vec<Real>> &query, const SearchOptions &opts, ResultSet<S> &result_set,
                             const DistanceMeasure<S, D, QS> &distance_measure, std::ifstream &dataset_ifs,
                             const vec<uint> *real_query_inds) const override {
        auto [file, num_channels, series_len, num_series] = RunSettings::get_instance().get_dataset_props();
        auto &logger = QueryLogger::get_instance();

        for (uint i = 0; i < num_series; ++i) {
            vec<vec<Real>> mts(num_channels);
            logger.start_timer(QC::IO_TIME_S);
            for (uint c = 0; c < num_channels; ++c) {
                if (!query[c].empty()) {
                    mts[c].resize(series_len);
                    dataset_ifs.read(reinterpret_cast<char *>(mts[c].data()), series_len * sizeof(Real));
                } else {
                    dataset_ifs.seekg(series_len * sizeof(Real), std::ios::cur);
                }
            }
            logger.stop_timer(QC::IO_TIME_S);

            logger.start_timer(QC::TS_EXAMINATION_TIME_S);
            distance_measure.update_result_set(result_set, {i, 0, series_len}, query, mts, real_query_inds);
            logger.stop_timer(QC::TS_EXAMINATION_TIME_S);

            logger.increment_count_col(QC::NUM_ENTRIES_EXAMINED);
        }
        return result_set.get_results();
    }
};

#endif  // SEQUENTIAL_SCAN_HPP

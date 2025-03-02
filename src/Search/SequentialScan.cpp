#include <fstream>

#include "Search/SequentialScan.hpp"
#include "Search/SearchMethod.hpp"
#include "Search/ResultSet.hpp"
#include "Search/Options/SearchOptions.hpp"
#include "Util/typedefs.hpp"
#include "Util/RunSettings.hpp"
#include "Util/Logger.hpp"

vec<SearchResult> SequentialScan::search(const vec<vec<float>> &query, const SearchOptions &opts,
                                         std::ifstream &dataset_ifs) const {
    auto [file, num_channels, series_len, num_series] = RunSettings::get_instance().get_dataset_props();
    auto &logger = QueryLogger::get_instance();

    for (uint i = 0; i < num_series; ++i) {
        vec<vec<float>> mts(num_channels);
        logger.start_timer(QC::IO_TIME_S);
        for (uint c = 0; c < num_channels; ++c) {
            if (!query[c].empty()) {
                mts[c].resize(series_len);
                dataset_ifs.read(reinterpret_cast<char *>(mts[c].data()), series_len * sizeof(float));
            } else {
                dataset_ifs.seekg(series_len * sizeof(float), std::ios::cur);
            }
        }
        logger.stop_timer(QC::IO_TIME_S);

        logger.start_timer(QC::TS_EXAMINATION_TIME_S);
        opts.distance_measure->update_result_set(opts.result_set.get(), {i, 0, series_len}, query, mts);
        logger.stop_timer(QC::TS_EXAMINATION_TIME_S);

        logger.increment_count_col(QC::NUM_ENTRIES_EXAMINED);
    }
    return opts.result_set.get()->get_results();
}

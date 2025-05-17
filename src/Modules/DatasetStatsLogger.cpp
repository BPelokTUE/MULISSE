#include "Util/Logging/DatasetStatsLogger.hpp"

#include "Util/RunSettings/RunSettings.hpp"

const str DatasetStatsLogger::DATASET_STATS_FILE = "dataset_stats.csv";

using DSTC = DatasetStatsColumn;

void DatasetStatsLogger::write_entry(const str &dataset_file, uint ts_ind, MtsNumChannelsT channel,
                                     const DatasetStats &stats) {
#ifndef DISABLE_LOGGING
    DatasetStatsLogger instance;
    auto &RS = RunSettings::get_instance();

    str dataset_stats_path = fs::path(RS.get_logs_path()) / DatasetStatsLogger::DATASET_STATS_FILE;
    instance.file_setup(dataset_stats_path, DATASET_STATS_COL_STRS);
    instance.write_row(dataset_stats_path,
                       {
                           {DSTC::DATASET_FILE, dataset_file},
                           {DSTC::TS_IND, to_string(ts_ind)},
                           {DSTC::CHANNEL, to_string(channel)},
                           {DSTC::MEAN, to_string(stats.m_mean)},
                           {DSTC::VARIANCE, to_string(stats.m_variance)},
                           {DSTC::SKEWNESS, to_string(stats.m_skewness)},
                           {DSTC::KURTOSIS, to_string(stats.m_kurtosis)},
                       },
                       DATASET_STATS_COL_ENUMS);
#endif  // DISABLE_LOGGING
}

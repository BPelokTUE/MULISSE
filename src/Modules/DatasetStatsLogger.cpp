#include "Util/Logging/DatasetStatsLogger.hpp"

#include "Util/Artefacts/MtsDataset.hpp"

const str DatasetStatsLogger::DATASET_STATS_FILE = "dataset_stats.csv";

using DSTC = DatasetStatsColumn;

DatasetStatsLogger::DatasetStatsLogger(const str &logs_path) {
    m_dataset_stats_path = fs::path(logs_path) / DATASET_STATS_FILE;
    file_setup(m_dataset_stats_path, DATASET_STATS_COL_STRS);
}

void DatasetStatsLogger::write_entry(const MtsDataset &dataset, uint ts_ind, MtsNumChannelsT channel,
                                     const DatasetStats &stats) {
#ifndef DISABLE_LOGGING
    write_row(m_dataset_stats_path,
              {
                  {DSTC::ID, to_string(determine_index(m_dataset_stats_path))},
                  {DSTC::DATASET_ID, to_string(dataset.get_log_id())},
                  {DSTC::DATASET_FILE, dataset.get_properties().m_dataset_path},
                  {DSTC::TS_IND, to_string(ts_ind)},
                  {DSTC::CHANNEL, to_string(channel)},
                  {DSTC::MEAN, to_string(stats.m_mean)},
                  {DSTC::STD, to_string(stats.m_std)},
                  {DSTC::SKEWNESS, to_string(stats.m_skewness)},
                  {DSTC::KURTOSIS, to_string(stats.m_kurtosis)},
                  {DSTC::TOTAL_VAR_MEANS, get_collection_str(stats.m_total_var_means)},
                  {DSTC::TOTAL_VAR_STDS, get_collection_str(stats.m_total_var_stds)},
                  {DSTC::AUTOCORR_MEANS, get_collection_str(stats.m_autocorr_means)},
                  {DSTC::AUTOCORR_STDS, get_collection_str(stats.m_autocorr_stds)},
              },
              DATASET_STATS_COL_ENUMS);
#endif  // DISABLE_LOGGING
}

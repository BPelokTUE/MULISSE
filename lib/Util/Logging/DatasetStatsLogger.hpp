#ifndef UTIL_LOGGING_DATASETSTATSLOGGER_HPP
#define UTIL_LOGGING_DATASETSTATSLOGGER_HPP

#include "Util/HelperFuncs/Enums.hpp"
#include "Util/Logging/Logger.hpp"
#include "Util/Types/Numbers.hpp"

class MtsDataset;

/** @brief Enum of the columns of the dataset statistics log file */
enum class DatasetStatsColumn {
    ID,               // ID of the entry in the dataset stats log
    DATASET_ID,       // ID of the dataset within the dataset log
    DATASET_FILE,     // Name of the dataset file
    TS_IND,           // Index of the time series
    CHANNEL,          // Channel index in the time series
    MEAN,             // Mean of the channel
    STD,              // Standard deviation of the channel
    SKEWNESS,         // Skewness of the channel
    KURTOSIS,         // Kurtosis of the channel
    TOTAL_VAR_MEANS,  // Means of total variances of the channel at different lags
    TOTAL_VAR_STDS,   // Standard deviations of total variances of the channel at different lags
    AUTOCORR_MEANS,   // Means of autocorrelations of the channel at different lags
    AUTOCORR_STDS,    // Standard deviations of autocorrelations of the channel at different lags
};

DEFINE_ENUM_CONSTS_NO_EXTRA(DatasetStatsColumn, DATASET_STATS_COL, false);

/** @brief Struct of dataset statistics */
struct DatasetStats {
    Real m_mean, m_std, m_skewness, m_kurtosis;
    vec<Real> m_total_var_means, m_total_var_stds, m_autocorr_means, m_autocorr_stds;
};

/** @brief Class for logging dataset statistics */
class DatasetStatsLogger : public Logger {
   public:
    DatasetStatsLogger(const str &logs_path);

    void write_entry(const MtsDataset &dataset, uint ts_ind, MtsNumChannelsT channel, const DatasetStats &stats);

    static const str DATASET_STATS_FILE;

    str m_dataset_stats_path;
};

#endif  // UTIL_LOGGING_DATASETSTATSLOGGER_HPP

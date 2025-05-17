#ifndef UTIL_LOGGING_DATASETSTATSLOGGER_HPP
#define UTIL_LOGGING_DATASETSTATSLOGGER_HPP

#include "Util/HelperFuncs/Enums.hpp"
#include "Util/Logging/Logger.hpp"
#include "Util/Types/Numbers.hpp"

/** @brief Enum of the columns of the dataset statistics log file */
enum class DatasetStatsColumn {
    DATASET_FILE,  // Name of the dataset file
    TS_IND,        // Index of the time series
    CHANNEL,       // Channel index in the time series
    MEAN,          // Mean of the channel
    VARIANCE,      // Variance of the channel
    SKEWNESS,      // Skewness of the channel
    KURTOSIS,      // Kurtosis of the channel
};

DEFINE_ENUM_CONSTS_NO_EXTRA(DatasetStatsColumn, DATASET_STATS_COL, false);

/** @brief Struct of dataset statistics */
struct DatasetStats {
    Real m_mean, m_variance, m_skewness, m_kurtosis;
};

/** @brief Class for logging dataset statistics */
class DatasetStatsLogger : public Logger {
   public:
    static void write_entry(const str &dataset_file, uint ts_ind, MtsNumChannelsT channel, const DatasetStats &stats);

    static const str DATASET_STATS_FILE;
};

#endif  // UTIL_LOGGING_DATASETSTATSLOGGER_HPP

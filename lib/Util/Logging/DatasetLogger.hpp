#ifndef DATASET_LOGGER_HPP
#define DATASET_LOGGER_HPP

#include "Util/HelperFuncs/Enums.hpp"
#include "Util/Logging/Logger.hpp"
#include "Util/Types/Numbers.hpp"
#include "Util/Types/Pointers.hpp"

struct MtsDataset;
struct RandomWalkGenOptions;
struct CsvDatasetGenOptions;

/** @brief Enum of the columns of the dataset settings log file */
enum class DatasetSettingsColumn {
    ID,              // ID of the setting within the log file
    DATASET_FILE,    // Name to the dataset file
    SERIES_LENGTH,   // Length of each time series
    NUM_CHANNELS,    // Number of channels
    NUM_SERIES,      // Number of time series in the dataset
    SD,              // The standard deviation of the Gaussian noise used for generating the random walk dataset
    MIN_SUBS_SD,     // Minimum standard deviation required for all subsequences when parsing CSV datasets
    SOURCE_CSVS,     // Source CSV files used for generating the CSV dataset
    L_MIN,           // Minimum length of subsequences that will be searched for (required for normalization)
    L_MAX,           // Maximum length of subsequences that will be searched for (required for normalization)
    SEED,            // The random seed to generate the dataset
    SIZE_ON_DISK_B,  // Size of the dataset file on disk in bytes
};

DEFINE_ENUM_CONSTS_NO_EXTRA(DatasetSettingsColumn, DATASET_SETTINGS_COL, false);

// DatasetLogger class

/** @brief Class for logging dataset settings */
class DatasetLogger : public Logger {
   public:
    /**
     * @brief Constructor
     * @param logs_path Path to the logs directory
     */
    DatasetLogger(const str &logs_path);

    /**
     * @brief Write the entry
     * @param rw_gen_opts Options used for generating the random walk dataset
     * @param dataset The MtsDataset
     * @return The ID of the entry within the log file, or 0 if logging is disabled
     */
    uint write_entry(const RandomWalkGenOptions &rw_gen_opts, const MtsDataset &dataset) const;

    /**
     * @brief Write the entry
     * @param csv_gen_opts Options used for generating the CSV dataset
     * @param dataset The MtsDataset
     * @return The ID of the entry within the log file, or 0 if logging is disabled
     */
    uint write_entry(const CsvDatasetGenOptions &csv_gen_opts, const MtsDataset &dataset) const;

   private:
    static const str DATASET_SETTINGS_FILE;

    str m_dataset_settings_path;
};

#endif  // DATASET_LOGGER_HPP

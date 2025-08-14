#ifndef DATASET_LOGGER_HPP
#define DATASET_LOGGER_HPP

#include "Util/HelperFuncs/Enums.hpp"
#include "Util/Logging/Logger.hpp"
#include "Util/Types/Numbers.hpp"
#include "Util/Types/Pointers.hpp"

struct MtsDatasetSettings;

/** @brief Enum of the columns of the dataset settings log file */
enum class DatasetSettingsColumn {
    ID,              // Index of the setting within the log file
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

// Dataset types

enum DatasetType { RANDOM_WALK, CSV };

struct IDatasetLogAttributes {
    virtual ~IDatasetLogAttributes() = default;

    /**
     * @brief Get the type of the dataset
     * @return The type of the dataset
     */
    virtual DatasetType get_type() = 0;
};

struct RandomWalkLogAttributes : IDatasetLogAttributes {
    /**
     * @brief Constructor for random walk dataset log attributes
     * @param noise The standard deviation of the Gaussian noise used for generating the random walk dataset
     * @param seed The random seed used for generating the dataset
     */
    RandomWalkLogAttributes(Real noise, int seed);

    DatasetType get_type() override;

    Real m_noise;
    int m_seed;
};

struct CsvDatasetLogAttributes : IDatasetLogAttributes {
    /**
     * @brief Constructor for CSV dataset log attributes
     * @param source_csvs The source CSV files used for generating the dataset
     * @param series_generated The number of time series generated from the CSV files
     * @param l_min The minimum length of subsequences to check standard deviation for
     * @param l_max The maximum length of subsequences to check standard deviation for
     * @param min_subs_sd The minimum standard deviation required for all subsequences
     * @param seed The random seed used for generating the dataset
     */
    CsvDatasetLogAttributes(const vec<str> &source_csvs, uint series_generated, uint l_min, uint l_max,
                            Real min_subs_sd, int seed);

    DatasetType get_type() override;

    uint m_series_generated, m_l_min, m_l_max;
    int m_seed;
    Real m_min_subs_sd;
    vec<str> m_source_csvs;
};

// DatasetLogger class

/** @brief Class for logging dataset settings */
class DatasetLogger : public Logger {
   public:
    DatasetLogger(const DatasetLogger &) = delete;
    DatasetLogger &operator=(const DatasetLogger &) = delete;

    /**
     * @brief Write the entry
     * @param attributes Attributes of the generated dataset
     * @param dataset_settings Settings of the dataset
     * @param logs_path Path to the logs directory
     */
    static void write_entry(uptr<IDatasetLogAttributes> attributes, const MtsDatasetSettings &dataset_settings,
                            const str &logs_path);

   private:
    DatasetLogger() = default;

    static const str DATASET_SETTINGS_FILE;
};

#endif  // DATASET_LOGGER_HPP

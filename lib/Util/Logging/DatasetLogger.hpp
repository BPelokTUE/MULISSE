#ifndef DATASET_LOGGER_HPP
#define DATASET_LOGGER_HPP

#include "Util/HelperFuncs/Enums.hpp"
#include "Util/Logging/Logger.hpp"
#include "Util/Types/Numbers.hpp"
#include "Util/Types/Pointers.hpp"

/** @brief Enum of the columns of the dataset settings log file */
enum class DatasetSettingsColumn {
    ID,             // Index of the setting within the log file
    DATASET_FILE,   // Name to the dataset file
    SERIES_LENGTH,  // Length of each time series
    NUM_CHANNELS,   // Number of channels
    NUM_SERIES,     // Number of time series in the dataset
    SD,             // The standard deviation of the Gaussian noise used for generating the random walk dataset
    SOURCE_CSVS,    // Source CSV files used for generating the CSV dataset
    L_MIN,          // Minimum length of subsequences that will be searched for (required for normalization)
    L_MAX,          // Maximum length of subsequences that will be searched for (required for normalization)
    SEED,           // The random seed to generate the dataset
};

DEFINE_ENUM_CONSTS_NO_EXTRA(DatasetSettingsColumn, DATASET_SETTINGS_COL, false);

// Dataset types

enum DatasetType { RANDOM_WALK, CSV };

struct IDatasetLogAttributes {
    virtual ~IDatasetLogAttributes() = default;

    virtual DatasetType get_type() = 0;
};

struct RandomWalkLogAttributes : IDatasetLogAttributes {
    RandomWalkLogAttributes(Real noise, int seed);

    DatasetType get_type() override;

    Real m_noise;
    int m_seed;
};

struct CsvDatasetLogAttributes : IDatasetLogAttributes {
    CsvDatasetLogAttributes(const vec<str> &source_csvs, uint series_generated, uint l_min, uint l_max, int seed);

    DatasetType get_type() override;

    vec<str> m_source_csvs;
    uint m_series_generated, m_l_min, m_l_max;
    int m_seed;
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
     * */
    static void write_entry(uptr<IDatasetLogAttributes> attributes);

   private:
    DatasetLogger() = default;

    static const str DATASET_SETTINGS_FILE;
};

#endif  // DATASET_LOGGER_HPP

#ifndef UTIL_LOGGING_FFTSLOGGER_HPP
#define UTIL_LOGGING_FFTSLOGGER_HPP

#include "Util/HelperFuncs/Enums.hpp"
#include "Util/Logging/Logger.hpp"

class MtsDatasetFfts;

/** @brief Enum of the columns of the FFTs settings log file */
enum class FftsSettingsColumn {
    ID,              // Index of the setting within the log file
    DATASET_ID,      // ID of the dataset
    DATASET_FILE,    // Name of the dataset file
    FFTS_FILE,       // Name of the FFTs file
    CALC_TIME_S,     // Time taken to calculate the FFTs in seconds
    SIZE_ON_DISK_B,  // Size of the index on disk in bytes
};

DEFINE_ENUM_CONSTS_NO_EXTRA(FftsSettingsColumn, FFTS_SETTINGS_COL, false);

// FftsLogger class

class FftsLogger : public Logger {
   public:
    /**
     * @brief Constructor
     * @param logs_path Path to the logs directory
     */
    FftsLogger(const str &logs_path);

    /**
     * @brief Write the entry
     * @param ffts The MtsFfts object to log
     */
    uint write_entry(const MtsDatasetFfts &ffts);

    /** @brief Start the timer for the FFT calculation */
    void start_timer();

    /** @brief Stop the timer for the FFT calculation */
    void stop_timer();

   private:
    static constexpr str FFTS_SETTINGS_FILE = "ffts_settings.csv";

    str m_ffts_settings_path;
    TimePoint m_calc_time_start;
    double m_calc_time_s = 0.0;
};

#endif  // UTIL_LOGGING_FFTSLOGGER_HPP

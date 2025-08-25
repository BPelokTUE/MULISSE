#ifndef UTIL_LOGGING_QUERYSETLOGGER_HPP
#define UTIL_LOGGING_QUERYSETLOGGER_HPP

struct MtsQuerySet;
struct QuerySetGenOptions;

#include "Util/HelperFuncs/Enums.hpp"
#include "Util/Logging/Logger.hpp"

/** @brief Enum of the columns of the query set settings log file */
enum class QuerySetSettingsColumn {
    ID,             // Index of the setting within the log file
    DATASET_ID,     // ID of the dataset the query set was generated from
    DATASET_FILE,   // Name of the dataset file the queries were generated from
    QUERY_FILE,     // Name of the query file
    NUM_QUERIES,    // Number of queries
    L_MIN,          // Minimum length of the queries
    L_MAX,          // Maximum length of the queries
    EXACT_LENGTHS,  // List of exact query lengths used to generate
    USED_CHANNELS,  // Number of channels used for the queries
    CHANNEL_MASK,   // Mask of channels used in the queries
    NOISE,          // The Gaussian noise added to the queries
    SEED,           // The random seed used to generate the queries
};

DEFINE_ENUM_CONSTS_NO_EXTRA(QuerySetSettingsColumn, QUERY_SET_SETTINGS_COL, false);

// QuerySetLogger class

/** @brief Class for logging query set settings */
class QuerySetLogger : public Logger {
   public:
    /**
     * @brief Constructor
     * @param logs_path Path to the logs directory
     */
    QuerySetLogger(const str &logs_path);

    /**
     * @brief Write the entry
     * @param query_set Query set to log
     * @param query_gen_opts Options used to generate the query set
     * @return The ID of the entry within the log file, or 0 if logging is disabled
     */
    uint write_entry(const MtsQuerySet &query_set, const QuerySetGenOptions &query_gen_opts);

   private:
    QuerySetLogger() = default;

    static constexpr str QUERY_SET_SETTINGS_FILE = "query_set_settings.csv";

    str m_query_set_settings_path;
};

#endif  // UTIL_LOGGING_QUERYSETLOGGER_HPP

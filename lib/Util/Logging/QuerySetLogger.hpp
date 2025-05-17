#ifndef QUERY_SET_LOGGER_HPP
#define QUERY_SET_LOGGER_HPP

#include "Search/QuerySetOptions.hpp"
#include "Util/HelperFuncs/Enums.hpp"
#include "Util/Logging/Logger.hpp"

/** @brief Enum of the columns of the query set settings log file */
enum class QuerySetSettingsColumn {
    ID,             // Index of the setting within the log file
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
    QuerySetLogger(const QuerySetLogger &) = delete;
    QuerySetLogger &operator=(const QuerySetLogger &) = delete;

    /**
     * @brief Write the entry
     * @param opts Options used to generate the query set
     */
    static void write_entry(QuerySetOptions &opts);

   private:
    QuerySetLogger() = default;

    static const str QUERY_SET_SETTINGS_FILE;
};

#endif  // QUERY_SET_LOGGER_HPP

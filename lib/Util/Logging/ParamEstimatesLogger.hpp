#ifndef UTIL_LOGGING_PARAMESTIMATELOGGER_HPP
#define UTIL_LOGGING_PARAMESTIMATELOGGER_HPP

#include "Util/HelperFuncs/Enums.hpp"
#include "Util/Logging/Logger.hpp"
#include "Util/Types/Numbers.hpp"

struct FlatEnvelopeParams;

enum class ParamEstimatesColumn {
    ID,            // Index of the estimate within the log file
    INDEX_FILE,    // Name of the index file for which the parameters are estimated
    L_PER_GROUP,   // Size of length groups (lambda) for the configuration
    POS_PER_ENV,   // Number of positions per envelope (gamma) for the configuration
    NUM_SEGMENTS,  // Number of segments (Ns) for the configuration
    SCORE,         // The score of the parameter estimate, should interpreted based on the type of estimator used
};

using PEC = ParamEstimatesColumn;

DEFINE_ENUM_CONSTS_NO_EXTRA(ParamEstimatesColumn, PARAM_ESTIMATES_COL, false);

class ParamEstimatesLogger : public Logger {
   public:
    ParamEstimatesLogger() = default;

    inline static ParamEstimatesLogger &get_instance() { return instance; };

    /** @brief Initialize the logger, creating the file if it does not exist */
    static void initialize();

    void write_entry(const FlatEnvelopeParams &params, Real score);

   private:
    uint m_estimate_id;
    str m_index_file, m_param_estimate_file_path;

    static ParamEstimatesLogger instance;
    static bool initialized;

    static const str PARAM_ESTIMATE_FILE;
};

#endif  // UTIL_LOGGING_PARAMESTIMATELOGGER_HPP

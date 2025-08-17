#ifndef UTIL_TYPES_RUNCONTEXT_HPP
#define UTIL_TYPES_RUNCONTEXT_HPP

#include "Util/Constants/Path.hpp"
#include "Util/Types/Numbers.hpp"
#include "Util/Types/String.hpp"

/** @brief Generic run context */
struct RunContext {
    /** @brief Whether subsequences are normalized */
    bool m_normalized = true;
    /** @brief Random seed to use */
    uint m_seed = 0;
    /** @brief Path to the data directory */
    str m_data_path = DEFAULT_DATA_PATH;
    /** @brief Path to the logs directory */
    str m_logs_path = DEFAULT_LOGS_PATH;
};

#endif UTIL_TYPES_RUNCONTEXT_HPP

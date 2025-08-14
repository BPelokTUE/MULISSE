#ifndef CLI_COMMONOPTIONS_HPP
#define CLI_COMMONOPTIONS_HPP

#include "Util/Constants/Path.hpp"
#include "Util/Types/Numbers.hpp"
#include "Util/Types/String.hpp"

/** @brief Common options for all subcommands */
struct CommonOptions {
    bool m_raw = false;
    uint m_seed = 0;
    str m_data_path = DEFAULT_DATA_PATH, m_logs_path = DEFAULT_LOGS_PATH;
};

#endif CLI_COMMONOPTIONS_HPP

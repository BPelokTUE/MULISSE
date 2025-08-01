#ifndef CLI_COMMONOPTIONS_HPP
#define CLI_COMMONOPTIONS_HPP

#include "Util/Types/Containers.hpp"
#include "Util/Types/Numbers.hpp"

struct CommonOptions {
    bool m_raw = false;
    uint m_seed = 0;
    str m_data_path = "../DATA", m_logs_path = "../LOGS";
};

#endif CLI_COMMONOPTIONS_HPP
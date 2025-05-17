#ifndef ENUMS_COMMANDTYPE_HPP
#define ENUMS_COMMANDTYPE_HPP

#include "Util/HelperFuncs/Enums.hpp"

/** @brief Enumeration type for the command type */
enum CommandType {
    CREATE_DS,
    PARSE_CSV,
    CREATE_QS,
    CALC_D_STATS,
    CALC_Q_STATS,
    INDEX,
    CALC_I_STATS,
    CALC_FFTS,
    SEARCH
};

DEFINE_ENUM_CONSTS_NO_EXTRA(CommandType, CMD_TYPE, false);

#endif  // ENUMS_COMMANDTYPE_HPP

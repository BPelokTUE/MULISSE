#ifndef ENUMS_INDEXTYPE_HPP
#define ENUMS_INDEXTYPE_HPP

#include <array>

#include "Util/HelperFuncs/Enums.hpp"

/** @brief Enumeration type for index types */
enum IndexType {
    ISAX,
    ISAX_ENVELOPE,
    ENVELOPE,
    SAX_ENVELOPE,
    ISAX_ENV_W_ENV,
    ISAX_ENV_W_SAX_ENV,
    TREE_ENVELOPE,
    BUCKETING_ENVELOPE,
    VL_ENVELOPE,
};

DEFINE_ENUM_CONSTS(IndexType, SEARCH_METHOD_TYPE, false,
                   (umap<str, IndexType>{
                       {"mulisse", ISAX_ENVELOPE},
                       {"variance_limiting", VL_ENVELOPE},
                       {"bucketing", BUCKETING_ENVELOPE},
                       {"bucketing_env", BUCKETING_ENVELOPE},
                   }));

constexpr std::array METHODS_W_ISAX{ISAX, ISAX_ENVELOPE, ISAX_ENV_W_ENV, ISAX_ENV_W_SAX_ENV};

constexpr std::array METHODS_W_SAX{ISAX,          ISAX_ENVELOPE,      SAX_ENVELOPE, ISAX_ENV_W_ENV, ISAX_ENV_W_SAX_ENV,
                                   TREE_ENVELOPE, BUCKETING_ENVELOPE, VL_ENVELOPE};

constexpr std::array METHODS_W_PAA{ISAX,          ISAX_ENVELOPE,      ENVELOPE,
                                   SAX_ENVELOPE,  ISAX_ENV_W_ENV,     ISAX_ENV_W_SAX_ENV,
                                   TREE_ENVELOPE, BUCKETING_ENVELOPE, VL_ENVELOPE};

constexpr std::array METHODS_W_ENVELOPE{ENVELOPE,           ISAX_ENVELOPE, SAX_ENVELOPE,       ISAX_ENV_W_ENV,
                                        ISAX_ENV_W_SAX_ENV, TREE_ENVELOPE, BUCKETING_ENVELOPE, VL_ENVELOPE};

constexpr std::array METHODS_W_FLAT_PART{ENVELOPE, SAX_ENVELOPE, ISAX_ENV_W_ENV, ISAX_ENV_W_SAX_ENV};

constexpr std::array METHODS_W_ENV_GROUPING{TREE_ENVELOPE, BUCKETING_ENVELOPE, VL_ENVELOPE};

constexpr std::array METHODS_W_ESTIMABLE_SIZE{ENVELOPE, SAX_ENVELOPE};

constexpr std::array FLAT_ENVELOPE_METHODS{ENVELOPE, SAX_ENVELOPE};

#endif  // ENUMS_INDEXTYPE_HPP

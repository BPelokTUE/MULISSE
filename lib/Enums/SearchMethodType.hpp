#ifndef ENUMS_SEARCHMETHODTYPE_HPP
#define ENUMS_SEARCHMETHODTYPE_HPP

#include <array>

#include "Util/HelperFuncs/Enums.hpp"

/** @brief Enumeration type for search methods */
enum SearchMethodType {
    ISAX,
    ISAX_ENVELOPE,
    ENVELOPE,
    SAX_ENVELOPE,
    ISAX_ENV_W_ENV,
    ISAX_ENV_W_SAX_ENV,
    TREE_ENVELOPE,
    VL_ENVELOPE,
    SEQUENTIAL_SCAN,
};

DEFINE_ENUM_CONSTS(SearchMethodType, SEARCH_METHOD_TYPE, false,
                   (umap<str, SearchMethodType>{
                       {"mulisse", ISAX_ENVELOPE},
                       {"scan", SEQUENTIAL_SCAN},
                       {"sequential", SEQUENTIAL_SCAN},
                       {"variance_limiting", VL_ENVELOPE},
                   }));

constexpr std::array METHODS_W_ISAX{ISAX, ISAX_ENVELOPE, ISAX_ENV_W_ENV, ISAX_ENV_W_SAX_ENV};

constexpr std::array METHODS_W_SAX{ISAX,          ISAX_ENVELOPE, SAX_ENVELOPE, ISAX_ENV_W_ENV, ISAX_ENV_W_SAX_ENV,
                                   TREE_ENVELOPE, VL_ENVELOPE};

constexpr std::array METHODS_W_PAA{ISAX,           ISAX_ENVELOPE,      ENVELOPE,      SAX_ENVELOPE,
                                   ISAX_ENV_W_ENV, ISAX_ENV_W_SAX_ENV, TREE_ENVELOPE, VL_ENVELOPE};

constexpr std::array METHODS_W_ENVELOPE{ENVELOPE,           ISAX_ENVELOPE, SAX_ENVELOPE, ISAX_ENV_W_ENV,
                                        ISAX_ENV_W_SAX_ENV, TREE_ENVELOPE, VL_ENVELOPE};

constexpr std::array METHODS_W_FLAT_PART{ENVELOPE, SAX_ENVELOPE, ISAX_ENV_W_ENV, ISAX_ENV_W_SAX_ENV};

constexpr std::array METHODS_W_ENV_GROUPING{TREE_ENVELOPE, VL_ENVELOPE};

#endif  // ENUMS_SEARCHMETHODTYPE_HPP

#ifndef SEARCH_METHOD_TYPE_HPP
#define SEARCH_METHOD_TYPE_HPP

#include <array>

#include "Util/typedefs.hpp"
#include "Util/utilities.hpp"

/** @brief Enumeration type for search methods */
enum SearchMethodType {
    ISAX,
    ISAX_ENVELOPE,
    ENVELOPE,
    SAX_ENVELOPE,
    ISAX_ENV_W_ENV,
    ISAX_ENV_W_SAX_ENV,
    TREE_ENVELOPE,
    SEQUENTIAL_SCAN,
};

DEFINE_ENUM_CONSTS(SearchMethodType, SEARCH_METHOD_TYPE, false,
                   (umap<str, SearchMethodType>{
                       {"mulisse", ISAX_ENVELOPE}, {"scan", SEQUENTIAL_SCAN}, {"sequential", SEQUENTIAL_SCAN}}));

constexpr std::array<SearchMethodType, 4> METHODS_W_ISAX{ISAX, ISAX_ENVELOPE, ISAX_ENV_W_ENV, ISAX_ENV_W_SAX_ENV};

constexpr std::array<SearchMethodType, 6> METHODS_W_SAX{ISAX,           ISAX_ENVELOPE,      SAX_ENVELOPE,
                                                        ISAX_ENV_W_ENV, ISAX_ENV_W_SAX_ENV, TREE_ENVELOPE};

constexpr std::array<SearchMethodType, 6> METHODS_W_ENVELOPE{ENVELOPE,       ISAX_ENVELOPE,      SAX_ENVELOPE,
                                                             ISAX_ENV_W_ENV, ISAX_ENV_W_SAX_ENV, TREE_ENVELOPE};

#endif  // SEARCH_METHOD_TYPE_HPP

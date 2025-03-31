#ifndef SEARCH_METHOD_TYPE_HPP
#define SEARCH_METHOD_TYPE_HPP

#include "Util/typedefs.hpp"
#include "Util/utilities.hpp"

/** @brief Enumeration type for search methods */
enum SearchMethodType { ISAX, ISAX_ENVELOPE, ENVELOPE, SAX_ENVELOPE, SEQUENTIAL_SCAN };

DEFINE_ENUM_CONSTS(SearchMethodType, SEARCH_METHOD_TYPE, false,
                   (umap<str, SearchMethodType>{
                       {"mulisse", ISAX_ENVELOPE}, {"scan", SEQUENTIAL_SCAN}, {"sequential", SEQUENTIAL_SCAN}}));

#endif  // SEARCH_METHOD_TYPE_HPP

#ifndef SEARCH_METHOD_TYPE_HPP
#define SEARCH_METHOD_TYPE_HPP

#include "Util/typedefs.hpp"
#include "Util/utilities.hpp"

/** @brief Enumeration type for search methods */
enum SearchMethodType { ISAX_ENVELOPE, SEQUENTIAL_SCAN };

DEFINE_ENUM_CONSTS(SearchMethodType, SEARCH_METHOD_TYPE, false,
                   (umap<str, SearchMethodType>{
                       {"isax", ISAX_ENVELOPE}, {"scan", SEQUENTIAL_SCAN}, {"sequential", SEQUENTIAL_SCAN}}));

/** @brief Vector of SearchMethodType values that refer to methods with envelopes */
const vec<SearchMethodType> ENVELOPE_METHODS = {ISAX_ENVELOPE};

#endif  // SEARCH_METHOD_TYPE_HPP

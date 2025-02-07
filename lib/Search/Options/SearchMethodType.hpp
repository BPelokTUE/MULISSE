#ifndef SEARCH_METHOD_TYPE_HPP
#define SEARCH_METHOD_TYPE_HPP

#include "Util/typedefs.hpp"
#include "Util/utilities.hpp"

/** @brief Enumeration type for search methods */
enum SearchMethodType { ISAX_ENVELOPE, SEQUENTIAL_SCAN };

/** @brief Map from strings to SearchMethodType */
const umap<str, SearchMethodType> STR_TO_SEARCH_METHOD_TYPE = {{"isax_envelope", ISAX_ENVELOPE},
                                                               {"isax", ISAX_ENVELOPE},
                                                               {"sequential_scan", SEQUENTIAL_SCAN},
                                                               {"scan", SEQUENTIAL_SCAN},
                                                               {"sequential", SEQUENTIAL_SCAN}};

/** @brief Vector of accepted strings for STR_TO_SEARCH_METHOD_TYPE */
const vec<str> SEARCH_METHOD_TYPE_STRS = get_map_keys(STR_TO_SEARCH_METHOD_TYPE);

/** @brief Vector of SearchMethodType values */
const vec<SearchMethodType> ENVELOPE_TYPES = {ISAX_ENVELOPE, SEQUENTIAL_SCAN};

#endif  // SEARCH_METHOD_TYPE_HPP

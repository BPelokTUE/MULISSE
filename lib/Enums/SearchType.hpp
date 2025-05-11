#ifndef SEARCH_TYPE_HPP
#define SEARCH_TYPE_HPP

#include "Util/HelperFuncs/Enums.hpp"

/** @brief Types of similarity search */
enum SearchType { KNN, R_RANGE };

DEFINE_ENUM_CONSTS_NO_EXTRA(SearchType, SEARCH_TYPE, false);

#endif  // SEARCH_TYPE_HPP

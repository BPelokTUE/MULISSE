#ifndef SEARCH_OPTIONS_HPP
#define SEARCH_OPTIONS_HPP

#include "Search/DistanceMeasure.hpp"
#include "Search/ResultSet.hpp"

/** @brief Types of similarity search */
enum SearchType { KNN, R_RANGE };

/** @brief Options for searching */
struct SearchOptions {
    /** @brief Whether to run exact or approximate search */
    bool exact = true;
    /** @brief Whether to Z-normalize or not */
    bool normalized = true;
    /** @brief Result set */
    uptr<IResultSet> result_set;
    /** @brief Distance measure to use */
    uptr<IDistanceMeasure> distance_measure;
};

#endif  // SEARCH_OPTIONS_HPP

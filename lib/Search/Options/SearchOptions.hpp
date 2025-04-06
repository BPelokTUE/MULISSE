#ifndef SEARCH_OPTIONS_HPP
#define SEARCH_OPTIONS_HPP

#include "Search/Options/DistanceType.hpp"
#include "Search/Options/SearchType.hpp"
#include "Search/Options/IndexOptions.hpp"
#include "Search/Options/SearchMethodType.hpp"
#include "Util/typedefs.hpp"

/** @brief Options for searching */
struct SearchOptions {
    /** @brief Whether to run exact or approximate search */
    bool exact = true;
    /** @brief Whether to Z-normalize or not */
    bool normalized = true;
    /** @brief Whether early abandoning is used (for Euclidean distance) */
    bool use_early_abandoning = true;
    /** @brief Whether query data points are sorted by absolute value (for Euclidean distance) */
    bool sort_queries = false;
    /** @brief Whether a priority queue is used for FlatEnvelopeIndexSearch */
    bool use_priority_queue = false;
    /** @brief Type of search method to use */
    SearchMethodType search_method_type;
    /** @brief Archive type of the index */
    ArchiveType index_format;
    /** @brief Type of search to execute (kNN or r-range) */
    SearchType search_type;
    /** @brief Type of distance to use */
    DistanceType distance_type;
    /** @brief k for kNN search */
    uint knn_k = 0;
    /** @brief Lengths per group, 0 by default, indicating no length-based grouping */
    uint lens_per_group = 0;
    /** @brief r for r-range search */
    Real r_range_r = 0.0;
    /** @brief Maximum number of leaves to visit if approximate search is used. Defaults to 0, indicating no max. */
    size_t max_leaves_to_visit = 0;
};

#endif  // SEARCH_OPTIONS_HPP

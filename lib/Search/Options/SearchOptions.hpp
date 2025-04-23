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
    bool m_exact = true;
    /** @brief Whether to Z-normalize or not */
    bool m_normalized = true;
    /** @brief Whether early abandoning is used (for Euclidean distance) */
    bool m_use_early_abandoning = true;
    /** @brief Whether query data points are sorted by absolute value (for Euclidean distance) */
    bool m_sort_queries = false;
    /** @brief Whether a priority queue is used for FlatEnvelopeIndexSearch */
    bool m_use_priority_queue = false;
    /** @brief Whether the index is grouped by length */
    bool m_group_by_length = false;
    /** @brief Type of search method to use */
    SearchMethodType m_search_method_type;
    /** @brief Archive type of the index */
    ArchiveType m_index_format;
    /** @brief Type of search to execute (kNN or r-range) */
    SearchType m_search_type;
    /** @brief Type of distance to use */
    DistanceType m_distance_type;
    /** @brief Minimum accepted query length (used for length-based grouping) */
    uint m_l_min;
    /** @brief Maximum accepted query length (used for length-based grouping) */
    uint m_l_max;
    /** @brief Lengths per group, default is 0, indicating no length-based grouping */
    uint m_l_per_group = 0;
    /** @brief k for kNN search */
    uint m_knn_k = 0;
    /** @brief r for r-range search */
    Real m_r_range_r = 0.0;
    /** @brief Maximum number of leaves to visit if approximate search is used. Defaults to 0, indicating no max. */
    size_t m_max_leaves_to_visit = 0;
};

#endif  // SEARCH_OPTIONS_HPP

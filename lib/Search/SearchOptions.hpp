#ifndef SEARCH_SEARCHOPTIONS_HPP
#define SEARCH_SEARCHOPTIONS_HPP

#include "Enums/ArchiveType.hpp"
#include "Enums/DistanceType.hpp"
#include "Enums/SearchMethodType.hpp"
#include "Enums/SearchType.hpp"
#include "Index/IndexParams.hpp"
#include "Util/Types/Numbers.hpp"

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
    /** @brief Whether to examine the whole series when distance calculation is performed */
    bool m_examine_whole = false;
    /** @brief Whether a priority queue is used for FlatEnvelopeIndexSearch */
    bool m_use_priority_queue = true;
    /** @brief Type of search method to use */
    SearchMethodType m_search_method_type;
    /** @brief Archive type of the index */
    ArchiveType m_index_format;
    /** @brief Type of search to execute (kNN or r-range) */
    SearchType m_search_type;
    /** @brief Type of distance to use */
    DistanceType m_distance_type;
    /** @brief k for kNN search */
    uint m_knn_k = 0;
    /** @brief r for r-range search */
    Real m_r_range_r = 0.0;
    /** @brief Maximum number of leaves to visit if approximate search is used. Defaults to 0, indicating no max. */
    size_t m_max_leaves_to_visit = 0;
    /** @brief SAX paramethers; Temporary solution for loading SAX breakpoints during search */
    SaxProperties m_sax_params;
};

#endif  // SEARCH_SEARCHOPTIONS_HPP

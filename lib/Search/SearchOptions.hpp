#ifndef SEARCH_OPTIONS_HPP
#define SEARCH_OPTIONS_HPP

#include "Search/DistanceMeasure.hpp"
#include "Search/ResultSet.hpp"
#include "Search/IndexOptions.hpp"

/** @brief Types of similarity search */
enum SearchType { KNN, R_RANGE };

/** @brief Map from strings to SearchType */
const umap<str, SearchType> STR_TO_SEARCH_TYPE = {{"knn", KNN}, {"r_range", R_RANGE}};

/** @brief Vector of accepted strings for STR_TO_SEARCH_TYPE */
const vec<str> SEARCH_TYPE_STRS = get_keys(STR_TO_SEARCH_TYPE);

/** @brief Options for searching */
struct SearchOptions {
    /** @brief Path to the index file to use */
    str index_path;
    /** @brief Path to the file containing the queries to answer */
    str query_path;
    /** @brief Path to the file to save the results into */
    str results_path;
    /** @brief Type of index */
    IndexType index_type;
    /** @brief Archive type of the index */
    ArchiveType index_format;
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

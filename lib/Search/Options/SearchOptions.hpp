#ifndef SEARCH_OPTIONS_HPP
#define SEARCH_OPTIONS_HPP

#include "Search/DistanceMeasure.hpp"
#include "Search/ResultSet.hpp"
#include "Search/Options/IndexOptions.hpp"
#include "Search/Options/SearchMethodType.hpp"

/** @brief Types of similarity search */
enum SearchType { KNN, R_RANGE };

DEFINE_ENUM_CONSTS_NO_EXTRA(SearchType, SEARCH_TYPE, false);

/** @brief Options for searching */
struct SearchOptions {
    /** @brief Path to the index file to use if any */
    str index_path;
    /** @brief Path to the dataset file to use; Assumed to be the source of the index */
    str dataset_path;
    /** @brief Path to the file containing the queries to answer */
    str query_path;
    /** @brief Path to the file to save the results into */
    str results_path;
    /** @brief Type of search method to use */
    SearchMethodType search_method_type;
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

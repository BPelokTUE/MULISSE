#ifndef SEARCH_OPTIONS_HPP
#define SEARCH_OPTIONS_HPP

#include "Search/DistanceMeasure.hpp"
#include "Search/ResultSet.hpp"
#include "Search/Options/IndexOptions.hpp"
#include "Search/Options/SearchMethodType.hpp"
#include "Util/typedefs.hpp"

/** @brief Options for searching */
struct SearchOptions {
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

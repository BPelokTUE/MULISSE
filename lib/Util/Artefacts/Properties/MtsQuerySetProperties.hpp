#ifndef UTIL_ARTEFACTS_PROPERTIES_MTSQUERYSETPROPERTIES_HPP
#define UTIL_ARTEFACTS_PROPERTIES_MTSQUERYSETPROPERTIES_HPP

#include "Util/Types/LengthRange.hpp"
#include "Util/Types/Numbers.hpp"
#include "Util/Types/String.hpp"

/** @brief Properties of a MtsQuerySet */
struct MtsQuerySetProperties {
    /** @brief Number of queries to generate */
    uint m_num_queries;
    /** @brief Allowed query length range */
    LengthRange m_length_range;
    /** @brief Path to the query_set */
    str m_query_set_path;
    /** @brief Path to the meta file of the dataset from which the query set is generated */
    str m_dataset_meta_path;
};

#endif  // UTIL_ARTEFACTS_PROPERTIES_MTSQUERYSETPROPERTIES_HPP

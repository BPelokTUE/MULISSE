#ifndef INDEX_INDEXGENOPTIONS_HPP
#define INDEX_INDEXGENOPTIONS_HPP

#include "Enums/ArchiveType.hpp"
#include "Enums/EntryInserterType.hpp"
#include "Enums/IndexType.hpp"
#include "Index/Estimator/EstimatorParams.hpp"
#include "Util/Types/Numbers.hpp"
#include "Util/Types/Pointers.hpp"

/** @brief Options for creating an index */
struct IndexGenOptions {
    /** @brief Whether to adapt the index properties to the dataset entries */
    bool m_adapt = false;
    /** @brief Type of inserter to use */
    EntryInserterType m_inserter_type;
    /** @brief Estimator parameters */
    uptr<EstimatorParams> m_estimator_params;
};

#endif  // INDEX_INDEXGENOPTIONS_HPP

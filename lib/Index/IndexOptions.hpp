#ifndef INDEX_INDEXOPTIONS_HPP
#define INDEX_INDEXOPTIONS_HPP

#include "Enums/ArchiveType.hpp"
#include "Enums/EntryInserterType.hpp"
#include "Enums/SearchMethodType.hpp"
#include "Index/IndexParams.hpp"
#include "Util/Types/Numbers.hpp"
#include "Util/Types/Pointers.hpp"

/** @brief Options for creating an index */
struct IndexOptions {
    /** @brief Whether to Z-normalize the subsequences */
    bool m_normalized;
    /** @brief Whether to adapt the index properties to the dataset entries */
    bool m_adapt;
    /** @brief Whether to use length groups */
    bool m_use_length_groups;
    /** @brief Number of channels of each series */
    MtsNumChannelsT m_num_channels;
    /** @brief The type of the index method to use */
    SearchMethodType m_index_method;
    /** @brief Format to save the index in */
    ArchiveType m_index_format;
    /** @brief Type of inserter to use */
    EntryInserterType m_inserter_type;
    /** @brief Minimum accepted query length */
    uint m_l_min;
    /** @brief Maximum accepted query length */
    uint m_l_max;
    /** @brief Length time series in the dataset */
    uint m_series_len;
    /** @brief Lengths per group */
    uint m_l_per_group;
    /** @brief Maximum size of the index as a ratio of the dataset size. Only implemented for FlatEnvelopeIndex with LG
     * segmentation strategy other than AdaptiveMultiLGSegmentationStrategy. */
    Real m_index_size_limit;
    /** @brief Unique pointer to the index parameters */
    uptr<IIndexParams> m_index_params;
};

#endif  // INDEX_INDEXOPTIONS_HPP

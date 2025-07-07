#ifndef INDEX_INDEXOPTIONS_HPP
#define INDEX_INDEXOPTIONS_HPP

#include "Enums/ArchiveType.hpp"
#include "Enums/EntryInserterType.hpp"
#include "Enums/SearchMethodType.hpp"
#include "Index/Estimator/EstimatorParams.hpp"
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
    /** @brief Estimator parameters */
    uptr<EstimatorParams> m_estimator_params;
    /** @brief Unique pointer to the index parameters */
    uptr<IIndexParams> m_index_params;

    /**
     * @brief Set the parameters of the FlatEnvelopeIndex : l_per_group, pos_per_env and num_segments
     * @param FlatEnvelopeParams The parameters to set
     */
    void set_flat_envelope_params(const FlatEnvelopeParams &params) {
        if (auto env_index_params = dynamic_cast<EnvelopeIndexParams *>(m_index_params.get())) {
            env_index_params->set_flat_envelope_params(params);
        } else {
            throw std::runtime_error("Index parameters cannot be set using FlatEnvelopeParams");
        }
        m_l_per_group = params.m_l_per_group;
    }
};

#endif  // INDEX_INDEXOPTIONS_HPP

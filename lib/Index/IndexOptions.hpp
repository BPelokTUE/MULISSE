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
    bool m_adapt = false;
    /** @brief Whether to use length groups */
    bool m_use_length_groups;
    /** @brief Number of channels of each series */
    MtsNumChannelsT m_num_channels;
    /** @brief Minimum accepted query length */
    uint m_l_min;
    /** @brief Maximum accepted query length */
    uint m_l_max;
    /** @brief Length time series in the dataset */
    uint m_series_len;
    /** @brief Lengths per group */
    uint m_l_per_group;
    /** @brief The type of the index method to use */
    SearchMethodType m_index_method;
    /** @brief Format to save the index in */
    ArchiveType m_index_format;
    /** @brief Type of inserter to use */
    EntryInserterType m_inserter_type;
    /** @brief Estimator parameters */
    uptr<EstimatorParams> m_estimator_params;
    /** @brief Unique pointer to the index parameters */
    uptr<IIndexParams> m_index_params;

    /**
     * @brief Set the parameters of the FlatEnvelopeIndex : l_per_group, pos_per_env and num_segments
     * @param EnvelopeParams The parameters to set
     */
    void set_flat_envelope_params(const EnvelopeParams &params) {
        if (auto env_index_params = dynamic_cast<EnvelopeIndexParams *>(m_index_params.get())) {
            env_index_params->set_flat_envelope_params(params);
        } else {
            throw std::runtime_error("Index parameters cannot be set using EnvelopeParams");
        }
        m_l_per_group = params.m_l_per_group;
    }

    /**
     * @brief Set positions per envelope for envelope-based indexes
     * @param pos_per_env The number of positions per envelope to set
     */
    void set_pos_per_env(uint pos_per_env) {
        if (auto env_index_params = dynamic_cast<EnvelopeIndexParams *>(m_index_params.get())) {
            env_index_params->m_pos_per_env = pos_per_env;
        } else {
            throw std::runtime_error("Index does not use pos_per_env");
        }
    }

    /**
     * @brief Set the number of segments for PAA segmentation-based indexes
     * @param num_segments The number of segments to set
     */
    void set_num_segments(SaxSegIndT num_segments) {
        if (auto paa_index_params = dynamic_cast<PaaIndexParams *>(m_index_params.get())) {
            paa_index_params->m_segmentation_params.m_num_segments = num_segments;
            if (m_estimator_params && m_estimator_params->m_config_generator_params) {
                if (auto rand_env_config_gen_params = dynamic_cast<RandomEnvConfigGeneratorParams *>(
                        m_estimator_params->m_config_generator_params.get())) {
                    rand_env_config_gen_params->m_num_segments_max = num_segments;
                }
            }
        } else {
            throw std::runtime_error("Index does not use num_segments");
        }
    }
};

#endif  // INDEX_INDEXOPTIONS_HPP

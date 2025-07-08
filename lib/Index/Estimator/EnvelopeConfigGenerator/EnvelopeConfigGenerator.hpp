#ifndef INDEX_ESTIMATOR_ENVELOPECONFIGGENERATOR_ENVELOPECONFIGGENERATOR_HPP
#define INDEX_ESTIMATOR_ENVELOPECONFIGGENERATOR_ENVELOPECONFIGGENERATOR_HPP

#include "Enums/SearchMethodType.hpp"
#include "Util/Types/Containers.hpp"
#include "Util/Types/Numbers.hpp"

struct EnvelopeParams;

class IEnvelopeConfigGenerator {
   public:
    virtual ~IEnvelopeConfigGenerator() = default;

    /**
     * @brief Generate EnvelopeParams suitable for the given index size limit.
     * @param index_type The type of the index to generate configurations for.
     * @param index_size_limit The maximum size of the index to generate configurations for as a ratio of the dataset
     * size.
     * @return A vector of EnvelopeParams configurations.
     */
    virtual vec<EnvelopeParams> generate_configurations(SearchMethodType index_type, Real index_size_limit) = 0;

   protected:
    /**
     * @brief Get the EnvelopeParams with the maximum possible pos_per_env (gamma) and the size of the resulting index
     * @param num_segments The number of segments in the index.
     * @param l_per_group_ratio The number of lengths per group in the index as a ratio of the query length range.
     * @param index_type The type of the index to generate configurations for.
     * @param index_size_limit The maximum size of the index to generate configurations for as a ratio of the dataset
     * size.
     * @return A pair containing the size of the index and the corresponding EnvelopeParams
     */
    std::pair<size_t, EnvelopeParams> get_envelope_params_and_size(SaxSegIndT num_segments, Real l_per_group_ratio,
                                                                   SearchMethodType index_type,
                                                                   Real index_size_limit) const;

    /**
     * @brief Get the size limit in bytes for the index.
     * @param index_size_limit The maximum size of the index to generate configurations for as a ratio of the dataset
     * size.
     * @return The size limit in bytes.
     */
    size_t get_bytes_limit(Real index_size_limit) const;
};

#endif  // INDEX_ESTIMATOR_ENVELOPECONFIGGENERATOR_ENVELOPECONFIGGENERATOR_HPP

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
};

#endif  // INDEX_ESTIMATOR_ENVELOPECONFIGGENERATOR_ENVELOPECONFIGGENERATOR_HPP

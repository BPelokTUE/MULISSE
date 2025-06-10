#ifndef INDEX_ESTIMATOR_CONFIGGENERATOR_CONFIGGENERATOR_HPP
#define INDEX_ESTIMATOR_CONFIGGENERATOR_CONFIGGENERATOR_HPP

#include "Util/Types/Containers.hpp"
#include "Util/Types/Numbers.hpp"

struct FlatEnvelopeParams;

class IConfigGenerator {
   public:
    virtual ~IConfigGenerator() = default;

    /**
     * @brief Generate FlatEnvelopeParams suitable for the given index size limit.
     * @param index_size_limit The maximum size of the index to generate configurations for as a ratio of the dataset
     * size.
     * @return A vector of FlatEnvelopeParams configurations.
     */
    virtual vec<FlatEnvelopeParams> generate_configurations(Real index_size_limit) = 0;
};

#endif  // INDEX_ESTIMATOR_CONFIGGENERATOR_CONFIGGENERATOR_HPP

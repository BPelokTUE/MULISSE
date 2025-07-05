#ifndef INDEX_ESTIMATOR_CONFIGGENERATOR_DUMMYCONFIGGENERATOR_HPP
#define INDEX_ESTIMATOR_CONFIGGENERATOR_DUMMYCONFIGGENERATOR_HPP

#include "Index/Estimator/ConfigGenerator/ConfigGenerator.hpp"

class DummyConfigGenerator : public IConfigGenerator {
   public:
    vec<FlatEnvelopeParams> generate_configurations(SearchMethodType index_type, Real index_size_limit) override;
};

#endif  // INDEX_ESTIMATOR_CONFIGGENERATOR_DUMMYCONFIGGENERATOR_HPP

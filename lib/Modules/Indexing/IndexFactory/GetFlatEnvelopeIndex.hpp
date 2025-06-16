#ifndef MODULES_INDEXING_INDEXFACTORY_GETFLATENVELOPEINDEX_HPP
#define MODULES_INDEXING_INDEXFACTORY_GETFLATENVELOPEINDEX_HPP

#include "Index/Index.hpp"
#include "Modules/Indexing/IndexFactory/IndexFactoryParams.hpp"

/**
 * @brief Get a FlatEnvelopeIndex instance
 * @tparam EnvT The type of envelope to store in the index, can be Envelope or SaxEnvelope
 * @param factory_params The parameters for the index factory
 */
template <typename EnvT>
sptr<IIndex<Envelope>> get_flat_envelope_index(IndexFactoryParams &factory_params);

#endif  // MODULES_INDEXING_INDEXFACTORY_GETFLATENVELOPEINDEX_HPP

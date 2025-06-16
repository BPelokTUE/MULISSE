#ifndef MODULES_INDEXING_INDEXFACTORY_GETTWOSTAGEENVELOPEINDEX_HPP
#define MODULES_INDEXING_INDEXFACTORY_GETTWOSTAGEENVELOPEINDEX_HPP

#include "Index/Index.hpp"
#include "Modules/Indexing/IndexFactory/IndexFactoryParams.hpp"

/**
 * @brief Get a two-stage iSAX with envelope index
 * @tparam EnvT The envelope type to use in the second stage, can be Envelope or SaxEnvelope
 */
template <typename EnvT>
sptr<IIndex<Envelope>> get_two_stage_isax_envelope_index(IndexFactoryParams &factory_params);

#endif  // MODULES_INDEXING_INDEXFACTORY_GETTWOSTAGEENVELOPEINDEX_HPP

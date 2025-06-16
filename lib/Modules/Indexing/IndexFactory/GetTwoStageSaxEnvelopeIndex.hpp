#ifndef MODULES_INDEXING_INDEXFACTORY_GETTWOSTAGESAXENVELOPEINDEX_HPP
#define MODULES_INDEXING_INDEXFACTORY_GETTWOSTAGESAXENVELOPEINDEX_HPP

#include "Index/Index.hpp"
#include "Modules/Indexing/IndexFactory/IndexFactoryParams.hpp"

sptr<IIndex<Envelope>> get_two_stage_isax_sax_envelope_index(IndexFactoryParams &factory_params);

#endif  // MODULES_INDEXING_INDEXFACTORY_GETTWOSTAGESAXENVELOPEINDEX_HPP

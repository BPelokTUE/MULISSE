#ifndef MODULES_INDEXING_INDEXFACTORY_GETFLATENVELOPEINDEX_HPP
#define MODULES_INDEXING_INDEXFACTORY_GETFLATENVELOPEINDEX_HPP

#include "Index/Index.hpp"
#include "Modules/Indexing/IndexFactory/IndexFactoryParams.hpp"

sptr<IIndex<Envelope>> get_flat_envelope_index(IndexFactoryParams &factory_params);

#endif  // MODULES_INDEXING_INDEXFACTORY_GETFLATENVELOPEINDEX_HPP

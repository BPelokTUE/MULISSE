#ifndef MODULES_INDEXING_INDEXFACTORY_GETTREEENVELOPEINDEX_HPP
#define MODULES_INDEXING_INDEXFACTORY_GETTREEENVELOPEINDEX_HPP

#include "Index/Index.hpp"
#include "Modules/Indexing/IndexFactory/IndexFactoryParams.hpp"

sptr<IIndex<Envelope>> get_envelope_tree_index(IndexFactoryParams &factory_params);

#endif  // MODULES_INDEXING_INDEXFACTORY_GETTREEENVELOPEINDEX_HPP

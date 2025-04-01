#ifndef SERIALIZATION_REGISTRATION_HPP
#define SERIALIZATION_REGISTRATION_HPP

#include <cereal/archives/binary.hpp>
#include <cereal/types/polymorphic.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/vector.hpp>

#include "Search/Index.hpp"
#include "Search/iSax/iSaxFinalizedIndex.hpp"
#include "Search/Envelope/EnvelopeIndex.hpp"
#include "Search/CombinedIndex.hpp"

// Register archive types
CEREAL_REGISTER_ARCHIVE(cereal::BinaryInputArchive)
CEREAL_REGISTER_ARCHIVE(cereal::JSONInputArchive)
CEREAL_REGISTER_ARCHIVE(cereal::BinaryOutputArchive)
CEREAL_REGISTER_ARCHIVE(cereal::JSONOutputArchive)

// Register base classes
CEREAL_REGISTER_TYPE(SeriesISaxProperties)
CEREAL_REGISTER_TYPE(SeriesISaxEnvelopeProperties)
CEREAL_REGISTER_POLYMORPHIC_RELATION(SeriesISaxProperties, SeriesISaxEnvelopeProperties)

// Register iSAX node types
CEREAL_REGISTER_TYPE(iSaxInternalNodeArgs<PaaTag>)
CEREAL_REGISTER_TYPE(iSaxInternalNodeArgs<EnvelopeTag>)

CEREAL_REGISTER_TYPE(iSaxEnvelopeInternalNodeArgs)
CEREAL_REGISTER_POLYMORPHIC_RELATION(iSaxInternalNodeArgs<PaaTag>, iSaxEnvelopeInternalNodeArgs)
CEREAL_REGISTER_POLYMORPHIC_RELATION(iSaxInternalNodeArgs<EnvelopeTag>, iSaxEnvelopeInternalNodeArgs)

CEREAL_REGISTER_TYPE(iSaxFinalizedInternal<PaaTag>)
CEREAL_REGISTER_TYPE(iSaxFinalizedInternal<EnvelopeTag>)
CEREAL_REGISTER_POLYMORPHIC_RELATION(iSaxFinalizedNode<PaaTag>, iSaxFinalizedInternal<PaaTag>)
CEREAL_REGISTER_POLYMORPHIC_RELATION(iSaxFinalizedNode<EnvelopeTag>, iSaxFinalizedInternal<EnvelopeTag>)

CEREAL_REGISTER_TYPE(iSaxFinalizedLeaf<PaaTag>)
CEREAL_REGISTER_TYPE(iSaxFinalizedLeaf<EnvelopeTag>)
CEREAL_REGISTER_POLYMORPHIC_RELATION(iSaxFinalizedNode<PaaTag>, iSaxFinalizedLeaf<PaaTag>)
CEREAL_REGISTER_POLYMORPHIC_RELATION(iSaxFinalizedNode<EnvelopeTag>, iSaxFinalizedLeaf<EnvelopeTag>)

// Dynamic initialization if needed
#ifdef CEREAL_DYNAMIC_INIT
CEREAL_REGISTER_DYNAMIC_INIT(SeriesISaxProperties)
CEREAL_REGISTER_DYNAMIC_INIT(iSaxFinalizedIndex)
CEREAL_REGISTER_DYNAMIC_INIT(FlatEnvelopeIndex)
CEREAL_REGISTER_DYNAMIC_INIT(CombinedFinalizedIndex)
#endif

#endif  // SERIALIZATION_REGISTRATION_HPP

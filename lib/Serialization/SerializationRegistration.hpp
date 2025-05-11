#ifndef SERIALIZATION_REGISTRATION_HPP
#define SERIALIZATION_REGISTRATION_HPP

#include <cereal/archives/binary.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/polymorphic.hpp>
#include <cereal/types/vector.hpp>

#include "Index/Entry/Envelope.hpp"
#include "Index/Entry/Paa.hpp"
#include "Index/EnvelopeIndex/Tree/EnvelopeNode.hpp"
#include "Index/iSaxIndex/FinalizedISaxIndex.hpp"
#include "Index/iSaxIndex/FinalizedISaxNode.hpp"

// Register archive types
CEREAL_REGISTER_ARCHIVE(cereal::BinaryInputArchive)
CEREAL_REGISTER_ARCHIVE(cereal::JSONInputArchive)
CEREAL_REGISTER_ARCHIVE(cereal::BinaryOutputArchive)
CEREAL_REGISTER_ARCHIVE(cereal::JSONOutputArchive)

// Register index entries
CEREAL_REGISTER_TYPE(Paa)
CEREAL_REGISTER_POLYMORPHIC_RELATION(IEntryData, Paa)
CEREAL_REGISTER_TYPE(Envelope)
CEREAL_REGISTER_POLYMORPHIC_RELATION(IEntryData, Envelope)

// Register iSAX node types
CEREAL_REGISTER_TYPE(iSaxInternalNodeArgs<PaaTag>)
CEREAL_REGISTER_TYPE(iSaxInternalNodeArgs<EnvelopeTag>)

CEREAL_REGISTER_TYPE(iSaxEnvelopeInternalNodeArgs)
CEREAL_REGISTER_POLYMORPHIC_RELATION(iSaxInternalNodeArgs<PaaTag>, iSaxEnvelopeInternalNodeArgs)
CEREAL_REGISTER_POLYMORPHIC_RELATION(iSaxInternalNodeArgs<EnvelopeTag>, iSaxEnvelopeInternalNodeArgs)

CEREAL_REGISTER_TYPE(iSaxFinalizedInternal<PaaTag>)
CEREAL_REGISTER_TYPE(iSaxFinalizedInternal<EnvelopeTag>)
CEREAL_REGISTER_POLYMORPHIC_RELATION(FinalizedISaxNode<PaaTag>, iSaxFinalizedInternal<PaaTag>)
CEREAL_REGISTER_POLYMORPHIC_RELATION(FinalizedISaxNode<EnvelopeTag>, iSaxFinalizedInternal<EnvelopeTag>)

CEREAL_REGISTER_TYPE(iSaxFinalizedLeaf<PaaTag>)
CEREAL_REGISTER_TYPE(iSaxFinalizedLeaf<EnvelopeTag>)
CEREAL_REGISTER_POLYMORPHIC_RELATION(FinalizedISaxNode<PaaTag>, iSaxFinalizedLeaf<PaaTag>)
CEREAL_REGISTER_POLYMORPHIC_RELATION(FinalizedISaxNode<EnvelopeTag>, iSaxFinalizedLeaf<EnvelopeTag>)

// Register envelope node types
CEREAL_REGISTER_TYPE(EnvelopeInternal)
CEREAL_REGISTER_POLYMORPHIC_RELATION(EnvelopeNode, EnvelopeInternal)

CEREAL_REGISTER_TYPE(EnvelopeLeaf)
CEREAL_REGISTER_POLYMORPHIC_RELATION(EnvelopeNode, EnvelopeLeaf)

// Register segmentation strategies
CEREAL_REGISTER_TYPE(UniformSegmentationStrategy)
CEREAL_REGISTER_POLYMORPHIC_RELATION(ISegmentationStrategy, UniformSegmentationStrategy)

CEREAL_REGISTER_TYPE(AdaptiveSegmentationStrategy)
CEREAL_REGISTER_POLYMORPHIC_RELATION(ISegmentationStrategy, AdaptiveSegmentationStrategy)

#endif  // SERIALIZATION_REGISTRATION_HPP

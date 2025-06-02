#include "Index/EnvelopeIndex/Tree/FinalizedTreeEnvelopeIndex.hpp"

#include "Index/Segmentation/ChannelSegmentationStrategy/ChannelSegmentationStrategy.hpp"

FinalizedTreeEnvelopeIndex::FinalizedTreeEnvelopeIndex(sptr<IChannelSegmentationStrategy> ch_segmentation_strategy,
                                                       const uint pos_per_env, vec<uptr<EnvelopeNode>> &&nodes)
    : FinalizedEnvelopeIndex(ch_segmentation_strategy, pos_per_env), m_first_layer_nodes(std::move(nodes)) {}

const vec<const EnvelopeNode *> FinalizedTreeEnvelopeIndex::get_first_layer_nodes() const {
    vec<const EnvelopeNode *> nodes(m_first_layer_nodes.size());
    for (size_t i = 0; i < nodes.size(); ++i) nodes[i] = m_first_layer_nodes[i].get();
    return nodes;
}

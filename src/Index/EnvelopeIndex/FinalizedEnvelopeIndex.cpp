#include "Index/EnvelopeIndex/FinalizedEnvelopeIndex.hpp"

#include "Index/Segmentation/ChannelSegmentationStrategy/ChannelSegmentationStrategy.hpp"

FinalizedEnvelopeIndex::FinalizedEnvelopeIndex(sptr<IChannelSegmentationStrategy> ch_segmentation_strategy,
                                               const uint pos_per_env)
    : m_ch_segmentation_strategy(ch_segmentation_strategy), m_pos_per_env(pos_per_env) {}

const IChannelSegmentationStrategy *FinalizedEnvelopeIndex::get_ch_segmentation_strategy() const {
    return m_ch_segmentation_strategy.get();
}

const uint FinalizedEnvelopeIndex::get_pos_per_env() const { return m_pos_per_env; }

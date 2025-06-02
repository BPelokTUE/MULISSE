#ifndef INDEX_ENVELOPEINDEX_FINALIZEDENVELOPEINDEX_HPP
#define INDEX_ENVELOPEINDEX_FINALIZEDENVELOPEINDEX_HPP

#include "Index/FinalizedIndex.hpp"
#include "Index/Traits/EntryTags.hpp"
#include "Util/Types/Pointers.hpp"

class IChannelSegmentationStrategy;

/** @brief Abstract base class for envelope-based finalized indexes */
class FinalizedEnvelopeIndex : public IFinalizedIndex<EnvelopeTag> {
   public:
    FinalizedEnvelopeIndex() = default;

    /**
     * @brief Construct a new FinalizedEnvelopeIndex instance
     * @param ch_segmentation_strategy The channel segmentation strategy to use
     * @param pos_per_env The number of positions per envelope
     */
    FinalizedEnvelopeIndex(sptr<IChannelSegmentationStrategy> ch_segmentation_strategy, const uint pos_per_env);

    /**
     * @brief Get the segmentation strategy
     * @return The channel segmentation strategy
     */
    const IChannelSegmentationStrategy *get_ch_segmentation_strategy() const;

    /**
     * @brief Get the number of positions per envelope
     * @return The number of positions per envelope
     */
    const uint get_pos_per_env() const;

   protected:
    uint m_pos_per_env;
    sptr<IChannelSegmentationStrategy> m_ch_segmentation_strategy;
};

#endif  // INDEX_ENVELOPEINDEX_FINALIZEDENVELOPEINDEX_HPP

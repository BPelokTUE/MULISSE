#ifndef INDEX_ENVELOPEINDEX_FINALIZEDENVELOPEINDEX_HPP
#define INDEX_ENVELOPEINDEX_FINALIZEDENVELOPEINDEX_HPP

#include "Index/FinalizedIndex.hpp"
#include "Index/Segmentation/ChannelSegmentationStrategy/ChannelSegmentationStrategy.hpp"
#include "Index/Traits/EntryTags.hpp"
#include "Util/Types/Pointers.hpp"

/** @brief Abstract base class for envelope-based finalized indexes */
class FinalizedEnvelopeIndex : public IFinalizedIndex<EnvelopeTag> {
   public:
    FinalizedEnvelopeIndex() = default;

    /**
     * @brief Construct a new FinalizedEnvelopeIndex instance
     * @param ch_segmentation_strategy The channel segmentation strategy to use
     * @param pos_per_env The number of positions per envelope
     */
    FinalizedEnvelopeIndex(sptr<IChannelSegmentationStrategy> ch_segmentation_strategy, const uint pos_per_env)
        : m_ch_segmentation_strategy(ch_segmentation_strategy), m_pos_per_env(pos_per_env) {}

    /**
     * @brief Get the segmentation strategy
     * @return The channel segmentation strategy
     */
    inline const IChannelSegmentationStrategy *get_ch_segmentation_strategy() const {
        return m_ch_segmentation_strategy.get();
    }

    /**
     * @brief Get the number of positions per envelope
     * @return The number of positions per envelope
     */
    inline const uint get_pos_per_env() const { return m_pos_per_env; }

   protected:
    uint m_pos_per_env;
    sptr<IChannelSegmentationStrategy> m_ch_segmentation_strategy;
};

#endif  // INDEX_ENVELOPEINDEX_FINALIZEDENVELOPEINDEX_HPP

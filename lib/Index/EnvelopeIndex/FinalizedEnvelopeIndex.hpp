#ifndef INDEX_ENVELOPEINDEX_FINALIZEDENVELOPEINDEX_HPP
#define INDEX_ENVELOPEINDEX_FINALIZEDENVELOPEINDEX_HPP

#include "Index/FinalizedIndex.hpp"
#include "Index/Segmentation/SegmentationStrategy.hpp"
#include "Index/Traits/FinalizedTraits.hpp"
#include "Util/Types/Pointers.hpp"

/** @brief Abstract base class for envelope-based finalized indexes */
class FinalizedEnvelopeIndex : public IFinalizedIndex<EnvelopeTag> {
   public:
    FinalizedEnvelopeIndex() = default;

    /**
     * @brief Construct a new FinalizedEnvelopeIndex instance
     * @param segmentation_strategy The segmentation strategy to use
     * @param pos_per_env The number of positions per envelope
     */
    FinalizedEnvelopeIndex(sptr<ISegmentationStrategy> segmentation_strategy, const uint pos_per_env)
        : m_segmentation_strategy(segmentation_strategy), m_pos_per_env(pos_per_env) {}

    /**
     * @brief Get the segmentation strategy
     * @return The segmentation strategy
     */
    inline const ISegmentationStrategy *get_segmentation_strategy() const { return m_segmentation_strategy.get(); }

    /**
     * @brief Get the number of positions per envelope
     * @return The number of positions per envelope
     */
    inline const uint get_pos_per_env() const { return m_pos_per_env; }

   protected:
    uint m_pos_per_env;
    sptr<ISegmentationStrategy> m_segmentation_strategy;
};

#endif  // INDEX_ENVELOPEINDEX_FINALIZEDENVELOPEINDEX_HPP

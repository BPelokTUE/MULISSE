#ifndef INDEX_ENVELOPEINDEX_ENVELOPEINDEX_HPP
#define INDEX_ENVELOPEINDEX_ENVELOPEINDEX_HPP

#include "Index/Entry/Envelope.hpp"
#include "Index/Entry/IndexEntry.hpp"
#include "Index/Index.hpp"

class IChannelSegmentationStrategy;

/** @brief Abstract base class for envelope-based indexes */
class EnvelopeIndex : public IIndex<Envelope> {
   public:
    EnvelopeIndex() = default;

    /**
     * @brief Initialize the parameters of the EnvelopeIndex
     * @param ch_segmentation_strategy The segmentation strategy to use
     * @param pos_per_env The number of positions per envelope
     */
    EnvelopeIndex(sptr<IChannelSegmentationStrategy> ch_segmentation_strategy, const uint pos_per_env);

    void insert(IndexEntry<Envelope> &entry) override;

    /**
     * @brief Get the entries of the index
     * @return The entries of the index
     */
    const vec<IndexEntry<Envelope>> &get_entries() const;

   protected:
    uint m_pos_per_env;
    sptr<IChannelSegmentationStrategy> m_ch_segmentation_strategy;
    vec<IndexEntry<Envelope>> m_entries;
};

#endif  // INDEX_ENVELOPEINDEX_ENVELOPEINDEX_HPP

#ifndef INDEX_ENVELOPEINDEX_FLAT_FINALIZEDFLATENVELOPEINDEX_HPP
#define INDEX_ENVELOPEINDEX_FLAT_FINALIZEDFLATENVELOPEINDEX_HPP

#include "Index/Entry/Envelope.hpp"
#include "Index/Entry/IndexEntry.hpp"
#include "Index/EnvelopeIndex/FinalizedEnvelopeIndex.hpp"
#include "Index/Segmentation/ChannelSegmentationStrategy/ChannelSegmentationStrategy.hpp"
#include "Serialization/Macros.hpp"

class FinalizedFlatEnvelopeIndex : public FinalizedEnvelopeIndex {
   public:
    FinalizedFlatEnvelopeIndex() = default;

    /**
     * @brief Construct a new FinalizedFlatEnvelopeIndex instance
     * @param ch_segmentation_strategy The channel segmentation strategy to use
     * @param pos_per_env The number of positions per envelope
     * @param entries The envelope entries in the index
     */
    FinalizedFlatEnvelopeIndex(sptr<IChannelSegmentationStrategy> ch_segmentation_strategy, const uint pos_per_env,
                               vec<IndexEntry<Envelope>> &&entries);

    const vec<IndexEntry<Envelope>> &get_entries() const;

   private:
    vec<IndexEntry<Envelope>> m_entries;

    MAKE_SERIALIZABLE((m_ch_segmentation_strategy, m_pos_per_env, m_entries));
};

#endif  // INDEX_ENVELOPEINDEX_FLAT_FINALIZEDFLATENVELOPEINDEX_HPP

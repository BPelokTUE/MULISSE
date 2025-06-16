#ifndef INDEX_ENVELOPEINDEX_ENVELOPEINDEX_HPP
#define INDEX_ENVELOPEINDEX_ENVELOPEINDEX_HPP

#include "Index/Entry/Envelope.hpp"
#include "Index/Entry/IndexEntry.hpp"
#include "Index/Index.hpp"

class IChannelSegmentationStrategy;

/**
 * @brief Abstract base class for envelope-based indexes
 * @param EnvT The type of envelope to store in the index, can be Envelope or SaxEnvelope
 * */
template <typename EnvT>
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
     * @brief Get the entry of the index at the given index
     * @param ind The index of the entry to get
     * @return The entry at the given index
     */
    auto get_entry(uint ind) const -> decltype(auto);

    /**
     * @brief Get the number of entries in the index
     * @return The number of entries in the index
     */
    const uint size() const;

   protected:
    uint m_pos_per_env;
    sptr<IChannelSegmentationStrategy> m_ch_segmentation_strategy;
    vec<IndexEntry<EnvT>> m_entries;
    const vec<Real> *m_breakpoints = nullptr;
};

#endif  // INDEX_ENVELOPEINDEX_ENVELOPEINDEX_HPP

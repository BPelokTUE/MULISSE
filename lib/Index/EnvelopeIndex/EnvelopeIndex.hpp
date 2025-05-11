#ifndef INDEX_ENVELOPEINDEX_ENVELOPEINDEX_HPP
#define INDEX_ENVELOPEINDEX_ENVELOPEINDEX_HPP

#include <memory>

#include "Index/Entry/Envelope.hpp"
#include "Index/Index.hpp"

/** @brief Abstract base class for envelope-based indexes */
class EnvelopeIndex : public IIndex<Envelope> {
   public:
    EnvelopeIndex() = default;

    /**
     * @brief Initialize the parameters of the EnvelopeIndex
     * @param segmentation_strategy The segmentation strategy to use
     * @param pos_per_env The number of positions per envelope
     */
    EnvelopeIndex(sptr<ISegmentationStrategy> segmentation_strategy, const uint pos_per_env)
        : m_segmentation_strategy(segmentation_strategy), m_pos_per_env(pos_per_env) {}

    void insert(IndexEntry<Envelope> &entry) override { m_entries.push_back(entry); }

    /**
     * @brief Get the entries of the index
     * @return The entries of the index
     */
    const vec<IndexEntry<Envelope>> &get_entries() const { return m_entries; }

   protected:
    uint m_pos_per_env;
    sptr<ISegmentationStrategy> m_segmentation_strategy;
    vec<IndexEntry<Envelope>> m_entries;
};

#endif  // INDEX_ENVELOPEINDEX_ENVELOPEINDEX_HPP

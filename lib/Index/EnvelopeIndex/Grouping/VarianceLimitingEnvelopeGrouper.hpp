#ifndef INDEX_ENVELOPEINDEX_GROUPING_VARIANCELIMITINGENVELOPEGROUPER_HPP
#define INDEX_ENVELOPEINDEX_GROUPING_VARIANCELIMITINGENVELOPEGROUPER_HPP

#include "Index/EnvelopeIndex/Grouping/EnvelopeGrouper.hpp"

/** @brief IEnvelopGrouper implementation that merges subsequent envelope entries until the update to the intra-series
 * variance estimated by the mean width of the envelope segments, exceeds a specified threshold */
class VarianceLimitingEnvelopeGrouper : public IEnvelopeGrouper {
   public:
    /**
     * @brief Construct a new VarianceLimitingEnvelopeGrouper instance
     * @param max_width_change Maximum allowed change to the mean envelope width
     */
    VarianceLimitingEnvelopeGrouper(Real max_width_change);

    vec<uptr<EnvelopeNode>> group_envelope_entries(vec<IndexEntry<Envelope>> &envelope_entries) override;

   private:
    Real m_max_width_change;
};

#endif  // INDEX_ENVELOPEINDEX_GROUPING_VARIANCELIMITINGENVELOPEGROUPER_HPP

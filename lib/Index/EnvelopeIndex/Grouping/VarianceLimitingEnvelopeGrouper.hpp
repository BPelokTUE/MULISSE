#ifndef INDEX_ENVELOPEINDEX_GROUPING_VARIANCELIMITINGENVELOPEGROUPER_HPP
#define INDEX_ENVELOPEINDEX_GROUPING_VARIANCELIMITINGENVELOPEGROUPER_HPP

#include "Index/EnvelopeIndex/Grouping/EnvelopeGrouper.hpp"

class VarianceLimitingEnvelopeGrouper : virtual public IEnvelopeGrouper {
    VarianceLimitingEnvelopeGrouper(Real max_width_update);

    vec<uptr<EnvelopeNode>> group_envelope_entries(vec<IndexEntry<Envelope>> &envelope_entries) override;

   private:
    Real m_max_width_update;
};

#endif  // INDEX_ENVELOPEINDEX_GROUPING_VARIANCELIMITINGENVELOPEGROUPER_HPP

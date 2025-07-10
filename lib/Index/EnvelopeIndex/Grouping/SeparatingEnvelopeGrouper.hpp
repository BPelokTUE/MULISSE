#ifndef INDEX_ENVELOPEINDEX_GROUPING_SEPARATINGENVELOPEGROUPER_HPP
#define INDEX_ENVELOPEINDEX_GROUPING_SEPARATINGENVELOPEGROUPER_HPP

#include "Index/EnvelopeIndex/Grouping/EnvelopeGrouper.hpp"

/** @brief IEnvelopeGrouper that applies another grouper per time series instead of on the whole dataset */
class SeparatingEnvelopeGrouper : public IEnvelopeGrouper {
   public:
    SeparatingEnvelopeGrouper(uptr<IEnvelopeGrouper> extra_grouper);

    vec<uptr<EnvelopeNode>> group_envelope_entries(vec<IndexEntry<Envelope>>::iterator entries_begin,
                                                   vec<IndexEntry<Envelope>>::iterator entries_end) override;

   private:
    uptr<IEnvelopeGrouper> m_extra_grouper;
};

#endif  // INDEX_ENVELOPEINDEX_GROUPING_SEPARATINGENVELOPEGROUPER_HPP

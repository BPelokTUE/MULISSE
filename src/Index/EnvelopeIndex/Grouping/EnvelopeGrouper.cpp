#include "Index/EnvelopeIndex/Grouping/EnvelopeGrouper.hpp"

#include "Index/Entry/Envelope.hpp"
#include "Index/Entry/IndexEntry.hpp"

vec<uptr<EnvelopeNode>> IEnvelopeGrouper::group_envelope_entries(vec<IndexEntry<Envelope>> &envelope_entries) {
    return group_envelope_entries(envelope_entries.begin(), envelope_entries.end());
}

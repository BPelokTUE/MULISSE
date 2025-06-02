#include "Index/EnvelopeIndex/Grouping/InvSaxSortingBucketingEnvelopeGrouper.hpp"

vec<uptr<EnvelopeNode>> InvSaxSortingBucketingEnvelopeGrouper::group_envelope_entries(
    vec<IndexEntry<Envelope>> &envelope_entries) {
    sort_envelope_entries(envelope_entries);
    return BucketingEnvelopeGrouper::group_envelope_entries(envelope_entries);
}

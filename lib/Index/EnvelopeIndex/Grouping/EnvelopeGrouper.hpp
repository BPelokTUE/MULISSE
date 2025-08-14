#ifndef ENVELOPE_GROUPER_HPP
#define ENVELOPE_GROUPER_HPP

#include "Index/EnvelopeIndex/Tree/EnvelopeNode.hpp"
#include "Util/Types/Vec.hpp"

/** @brief Interface for envelope groupers */
class IEnvelopeGrouper {
   public:
    virtual ~IEnvelopeGrouper() = default;

    /**
     * @brief Group envelope_entries into a tree structure
     * @param entries_begin Iterator to the beginning of the envelope entries
     * @param entries_end Iterator to the end of the envelope entries
     * @return Vector of first layer envelope nodes
     */
    virtual vec<uptr<EnvelopeNode>> group_envelope_entries(vec<IndexEntry<Envelope>>::iterator entries_begin,
                                                           vec<IndexEntry<Envelope>>::iterator entries_end) = 0;

    /**
     * @brief Group envelope_entries into a tree structure
     * @param envelope_entries Vector of envelope entries to be grouped
     * @return Vector of first layer envelope nodes
     */
    vec<uptr<EnvelopeNode>> group_envelope_entries(vec<IndexEntry<Envelope>> &envelope_entries);
};

#endif  // ENVELOPE_GROUPER_HPP

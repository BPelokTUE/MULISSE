#ifndef ENVELOPE_GROUPER_HPP
#define ENVELOPE_GROUPER_HPP

#include "Index/EnvelopeIndex/Tree/EnvelopeNode.hpp"
#include "Util/Types/Containers.hpp"

/** @brief Interface for envelope groupers */
class IEnvelopeGrouper {
   public:
    virtual ~IEnvelopeGrouper() = default;

    /**
     * @brief Group envelope_entries into a tree structure
     * @param envelope_entries Vector of envelope_entries to group
     * @return Vector of first layer envelope nodes
     */
    virtual vec<uptr<EnvelopeNode>> group_envelope_entries(vec<IndexEntry<Envelope>> &envelope_entries) = 0;
};

#endif  // ENVELOPE_GROUPER_HPP

#ifndef INDEX_ENTRY_SAXENVELOPE_HPP
#define INDEX_ENTRY_SAXENVELOPE_HPP

#include "Index/Entry/EntryData.hpp"
#include "Util/Constants/Math.hpp"

class ILengthGroupSegmentationStrategy;

class Envelope;

/** @brief SAX-discretized version of Envelope */
struct SaxEnvelope {
    /** @brief Lower bounds of the envelope */
    vec<SaxSymbolT> m_lower;
    /** @brief Upper bounds of the envelope */
    vec<SaxSymbolT> m_upper;

    SaxEnvelope() = default;

    /**
     * @brief Construct a new Envelope object
     * @param envelope The envelope to discretize
     * @param breakpoints The SAX breakpoints to use
     * @param num_bits The number of bits to use for the symbols
     */
    SaxEnvelope(Envelope envelope, const vec<Real> &breakpoints, SaxNumBitsT num_bits);

    /**
     * @brief Un-discretize the SAX envelope to a regular envelope
     * @param breakpoints The SAX breakpoints to use
     * @return The un-discretized envelope
     */
    Envelope to_envelope(const vec<Real> &breakpoints) const;

    /**
     * @brief Equality operator
     * @param other The other envelope to compare with
     */
    bool operator==(const SaxEnvelope &other) const;

    inline size_t size() const { return m_lower.size(); }

    void resize(size_t new_size);

    template <class Archive>
    void serialize(Archive &ar) {
        ar(m_lower, m_upper);
    }
};

#endif  // INDEX_ENTRY_SAXENVELOPE_HPP

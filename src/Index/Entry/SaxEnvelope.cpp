#include "Index/Entry/SaxEnvelope.hpp"

#include "Index/Entry/Envelope.hpp"
#include "Index/Sax/SaxWord.hpp"

SaxEnvelope::SaxEnvelope(Envelope envelope, const vec<Real> &breakpoints, SaxNumBitsT num_bits)
    : m_lower(SaxWord(envelope.m_lower, breakpoints, num_bits).m_symbols),
      m_upper(SaxWord(envelope.m_upper, breakpoints, num_bits).m_symbols) {}

Envelope SaxEnvelope::to_envelope(const vec<Real> &breakpoints) const {
    vec<Real> lower(m_lower.size()), upper(m_upper.size());
    for (size_t i = 0; i < m_lower.size(); ++i) {
        lower[i] = m_lower[i] == 0 ? -INF : breakpoints[m_lower[i] - 1];
        upper[i] = m_upper[i] == static_cast<SaxSegIndT>(breakpoints.size()) ? INF : breakpoints[m_upper[i]];
    }
    return Envelope(std::move(lower), std::move(upper));
}

void SaxEnvelope::resize(size_t new_size) {
    m_lower.resize(new_size);
    m_upper.resize(new_size);
}

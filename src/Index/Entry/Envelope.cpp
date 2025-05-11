#include "Index/Entry/Envelope.hpp"

Envelope::Envelope(vec<Real> lower, vec<Real> upper) : m_lower(std::move(lower)), m_upper(std::move(upper)) {}

bool Envelope::operator==(const Envelope &other) const { return m_lower == other.m_lower && m_upper == other.m_upper; }

void Envelope::resize(size_t new_size) {
    m_lower.resize(new_size);
    m_upper.resize(new_size);
}

const vec<Real> &Envelope::get_isax_input() const { return m_lower; }

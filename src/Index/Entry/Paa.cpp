#include "Index/Entry/Paa.hpp"

Paa::Paa(const vec<Real> &paa_values) : m_paa_values(paa_values) {}

bool Paa::operator==(const Paa &other) const { return m_paa_values == other.m_paa_values; }

size_t Paa::size() const { return m_paa_values.size(); }

void Paa::resize(size_t new_size) { m_paa_values.resize(new_size); }

const vec<Real> &Paa::get_isax_input() const { return m_paa_values; }

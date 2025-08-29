#include "Index/Sax/SaxBreakpoints.hpp"

SaxBreakpoints::SaxBreakpoints(SaxSymbolT alphabet_size = 0) : m_breakpoints(alphabet_size) {}

SaxBreakpoints::SaxBreakpoints(const vec<Real> &breakpoints) : m_breakpoints(breakpoints) {}

Real &SaxBreakpoints::operator[](size_t index) { return m_breakpoints[index]; }

const Real &SaxBreakpoints::operator[](size_t index) const { return m_breakpoints.at(index); }

const size_t SaxBreakpoints::size() const { return m_breakpoints.size(); }

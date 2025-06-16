#include "Index/Sax/SaxSymbolsFactory.hpp"

SaxSymbolsFactory::SaxSymbolsFactory(const BreakpointProperties &breakpoint_props, SaxNumBitsT sax_num_bits)
    : m_sax_num_bits(sax_num_bits),
      m_breakpoints(breakpoint_props.m_breakpoints),
      m_alphabet_num_bits(breakpoint_props.m_breakpoint_num_bits) {
    assert(m_sax_num_bits > 0);
    assert(m_breakpoints.size() >= (1 << m_sax_num_bits) - 1);
}

vec<SaxSymbolT> SaxSymbolsFactory::get_symbols(const vec<Real> &isax_input) {
    return std::move(SaxWord(isax_input, m_breakpoints, m_sax_num_bits, m_alphabet_num_bits).m_symbols);
}

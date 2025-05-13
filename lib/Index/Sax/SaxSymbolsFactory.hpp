#ifndef INDEX_SAX_SAXSYMBOLSFACTORY_HPP
#define INDEX_SAX_SAXSYMBOLSFACTORY_HPP

#include "Index/Sax/SaxWord.hpp"
#include "Util/RunSettings/BreakpointProperties.hpp"

class SaxSymbolsFactory {
   public:
    /**
     * @brief Construct a new SaxSymbolsFactory object
     * @param breakpoint_props The properties of the breakpoints
     * @param sax_num_bits The number of bits to use for the symbols
     */
    SaxSymbolsFactory(const BreakpointProperties &breakpoint_props, SaxNumBitsT sax_num_bits)
        : m_sax_num_bits(sax_num_bits),
          m_breakpoints(breakpoint_props.m_breakpoints),
          m_alphabet_num_bits(breakpoint_props.m_breakpoint_num_bits) {
        assert(m_sax_num_bits > 0);
        assert(m_breakpoints.size() >= (1 << m_sax_num_bits) - 1);
    }

    vec<SaxSymbolT> get_symbols(const vec<Real> &isax_input) {
        return std::move(SaxWord(isax_input, m_breakpoints, m_sax_num_bits, m_alphabet_num_bits).m_symbols);
    }

   private:
    SaxNumBitsT m_sax_num_bits, m_alphabet_num_bits;
    const vec<Real> &m_breakpoints;
};

#endif  // INDEX_SAX_SAXSYMBOLSFACTORY_HPP

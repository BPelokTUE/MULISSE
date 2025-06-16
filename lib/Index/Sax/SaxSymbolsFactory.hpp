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
    SaxSymbolsFactory(const BreakpointProperties &breakpoint_props, SaxNumBitsT sax_num_bits);

    vec<SaxSymbolT> get_symbols(const vec<Real> &isax_input);

   private:
    SaxNumBitsT m_sax_num_bits, m_alphabet_num_bits;
    const vec<Real> &m_breakpoints;
};

#endif  // INDEX_SAX_SAXSYMBOLSFACTORY_HPP

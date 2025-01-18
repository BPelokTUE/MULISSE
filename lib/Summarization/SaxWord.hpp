#ifndef SAX_WORD_HPP
#define SAX_WORD_HPP

#include "typedefs.hpp"

/**
 * @brief Symbolic Aggregate approXimation (SAX) word.
 */
class SaxWord {
   public:
    /**
     * @brief Construct a new iSaxWord object from the PAA of a time series.
     *
     * @param paa The Piecewise Aggregate Approximation (PAA) of a time series.
     * @param num_bits The number of bits to use for the symbols.
     * @param breakpoints The breakpoints to use for the symbols.
     */
    SaxWord(const vec<float>& paa, SaxNumBitsT num_bits, const vec<float>& breakpoints);

    SaxWord(const SaxWord&) = default;

    /**
     * @brief Construct a new SaxWord object.
     *
     * @param symbols The symbols of the word.
     * @param num_bits The number of bits to use for the symbols.
     */
    SaxWord(vec<SaxSymbolT> symbols, SaxNumBitsT num_bits) : m_symbols(symbols), m_max_num_bits(num_bits) {};

    /**
     * @brief Get the symbol at the given index.
     *
     * @param index The index of the symbol.
     * @return The symbol at the given index.
     */
    const SaxSymbolT& operator[](std::size_t index) const;

    /**
     * @brief Set the symbol at the given index without setting the number of bits.
     *
     * @param index The index of the symbol.
     * @param symbol The symbol to set.
     */
    void set_symbol(SaxSplitIndT index, SaxSymbolT symbol);

   protected:
    vec<SaxSymbolT> m_symbols;
    SaxNumBitsT m_max_num_bits;
};

#endif  // SAX_WORD_HPP

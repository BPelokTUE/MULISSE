#include <vector>

#include "typedefs.hpp"

/**
 * @brief indexable Symbolic Aggregate approXimation (iSAX) word.
 */
class iSaxWord {
   public:
    /**
     * @brief Construct a new iSaxWord object.
     *
     * @param paa The Piecewise Aggregate Approximation (PAA) of a time series.
     * @param start_num_bits The number of bits to use initially for all symbols.
     * @param breakpoints The breakpoints to use for the symbols.
     */
    iSaxWord(const vec<float>& paa, iSaxNumBitsT start_num_bits, vec<float> breakpoints);

    iSaxWord(const iSaxWord&) = default;

    /**
     * @brief Get the symbol at the given index.
     *
     * @param index The index of the symbol.
     * @return The symbol at the given index.
     */
    const iSaxSymbolT& operator[](std::size_t index) const;

    /**
     * @brief Set the symbol at the given index without setting the number of bits.
     *
     * @param index The index of the symbol.
     * @param symbol The symbol to set.
     */
    void set_symbol(iSaxSplitIndT index, iSaxSymbolT symbol);

    /**
     * @brief Set the symbol at the given index with the given number of bits.
     *
     * @param index The index of the symbol.
     * @param symbol The symbol to set.
     * @param bits The number of bits to use for the symbol.
     */
    void set_symbol(iSaxSplitIndT index, iSaxSymbolT symbol, iSaxNumBitsT bits);

    /**
     * @brief Get a pair of iSaxWords by splitting the current iSaxWord at the given index.
     *
     * @param index The index at which to split the iSaxWord.
     * @return A pair of iSaxWords.
     */
    std::pair<iSaxWord, iSaxWord> split(iSaxSplitIndT index) const;

   private:
    vec<iSaxSymbolT> m_symbols;
    vec<iSaxNumBitsT> m_num_bits;
    iSaxNumBitsT m_max_num_bits;
};

#ifndef ISAX_WORD_HPP
#define ISAX_WORD_HPP

#include <vector>

#include "Summarization/SaxWord.hpp"

/**
 * @brief indexable Symbolic Aggregate approXimation (iSAX) word.
 */
class iSaxWord : public SaxWord {
   public:
    /**
     * @brief Construct a new iSaxWord object.
     *
     * @param paa The Piecewise Aggregate Approximation (PAA) of a time series.
     * @param start_num_bits The number of bits to use initially for all symbols.
     * @param breakpoints The breakpoints to use for the symbols.
     */
    iSaxWord(const vec<float> &paa, SaxNumBitsT start_num_bits, const vec<float> &breakpoints);

    /**
     * @brief Copy constructor.
     *
     * @param other The iSaxWord to copy.
     */
    iSaxWord(const iSaxWord &) = default;

    /**
     * @brief Construct a new iSaxWord object.
     *
     * @param symbols The symbols of the word.
     * @param num_bits The number of bits to use for the symbols.
     * @param max_num_bits The maximum number of bits used for the symbols.
     */
    iSaxWord(vec<SaxSymbolT> symbols, vec<SaxNumBitsT> num_bits, SaxNumBitsT max_num_bits);

    /**
     * @brief Construct a new iSaxWord object.
     *
     * @param symbols The symbols of the word.
     * @param max_num_bits The number of bits to use for the symbols. All symbols will have the same number of bits.
     */
    iSaxWord(vec<SaxSymbolT> symbols, SaxNumBitsT max_num_bits);

    /**
     * @brief Get the number of bits used for the symbol at the given index.
     *
     * @param index The index of the symbol.
     * @return The number of bits used for the symbol at the given index.
     */
    const SaxNumBitsT &get_num_bits(SaxSplitIndT index) const;

    /**
     * @brief Set the symbol at the given index with the given number of bits.
     *
     * @param index The index of the symbol.
     * @param symbol The symbol to set.
     * @param bits The number of bits to use for the symbol.
     */
    void set_symbol_and_bits(SaxSplitIndT index, SaxSymbolT symbol, SaxNumBitsT bits);

    /**
     * @brief Apply a split to the word. Updates the iSAX word inplace.
     *
     * Updates the symbol specified by the split index after a split, based on the PAA and the breakpoints.
     *
     * @param paa The Piecewise Aggregate Approximation (PAA) of a time series.
     * @param breakpoints The breakpoints to use for the symbols.
     * @param split_ind The index of the symbol to update.
     * @return The bit that was appended to the symbol.
     */
    uint8_t apply_split(const vec<float> &paa, const vec<float> &breakpoints, SaxSplitIndT split_ind);

   private:
    vec<SaxNumBitsT> m_num_bits;
};

#endif  // ISAX_WORD_HPP

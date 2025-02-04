#ifndef ISAX_WORD_HPP
#define ISAX_WORD_HPP

#include <vector>

#include "Summarization/SaxWord.hpp"

/** @brief iSAX word settings */
struct iSaxWordSettings {
    /** @brief The number of bits to use for each segment */
    vec<SaxNumBitsT> num_bits;
    /** @brief The maximum number of bits in the alphabet */
    SaxNumBitsT alphabet_num_bits;
    /** @brief The vector of breakpoints for the symbol intervals */
    vec<float> breakpoints;
};

/** @brief indexable Symbolic Aggregate approXimation (iSAX) word */
class iSaxWord : public SaxWord {
   public:
    /**
     * @brief Constructor from the PAA of a time series
     *
     * @param paa The Piecewise Aggregate Approximation (PAA) of a time series
     * @param settings iSAX word settings containing the number of bits per symbol, the number
     *        of bits for the alphabet and the breakpoints
     */
    iSaxWord(const vec<float> &paa, const iSaxWordSettings &settings);

    iSaxWord() = default;

    /**
     * @brief Copy constructor
     *
     * @param other The iSaxWord to copy
     */
    iSaxWord(const iSaxWord &) = default;

    /**
     * @brief Constructor with the same number of bits for each segment
     *
     * @param symbols The symbols of the word
     * @param alphabet_num_bits The number of bits to use for the symbols. All symbols will have the same number of
     * bits.
     */
    iSaxWord(vec<SaxSymbolT> symbols, SaxNumBitsT alphabet_num_bits);

    /**
     * @brief Constructor with different number of bits per segment
     *
     * @param symbols The symbols of the word
     * @param num_bits The number of bits to use for the symbols
     * @param alphabet_num_bits The number of bits used by alphabet
     */
    iSaxWord(vec<SaxSymbolT> symbols, vec<SaxNumBitsT> num_bits, SaxNumBitsT alphabet_num_bits);

    /**
     * @brief Get the symbol at the given index
     *
     * @param index The index of the symbol
     * @return The symbol at the given index
     */
    SaxSymbolT operator[](std::size_t index) const override;

    /**
     * @brief Get the number of bits used for the symbol at the given index
     *
     * @param index The index of the symbol
     * @return The (reference to) the number of bits vector
     */
    const vec<SaxNumBitsT> &get_num_bits() const;

    /**
     * @brief Apply a split to the word; updates the iSAX word inplace
     *
     * Updates the symbol specified by the split index after a split. Assumes that the number of bits of the segment is
     * less than the alphabet's.
     *
     * @param split_ind The index of the symbol to update
     * @return The bit that was appended to the symbol
     */
    uint8_t apply_split(SaxSegIndT split_ind);

    /**
     * @brief Appends a bit to the symbol at the given index; updates the iSAX word inplace
     *
     * @param index The index of the symbol
     * @param bit The bit to append
     */
    void append_to_symbol(SaxSegIndT index, uint8_t bit);

    /**
     * @brief Removes a bit from the symbol at the given index; updates the iSAX word inplace
     *
     * @param index The index of the symbol
     * @param bit The bit to append
     */
    void remove_from_symbol(SaxSegIndT index);

    /**
     * @brief Set the symbol at the given index
     *
     * @param index The index of the symbol
     * @param num_bits Number of bits to use for the symbol
     * @param symbol Symbol (represented using `num_bits` bits)
     */
    void set_symbol(SaxSegIndT index, SaxNumBitsT num_bits, SaxSymbolT symbol);

    /**
     * @brief For each segment, set the symbols to the maximum of two words
     *
     * @param other The other iSaxWord to compare to. Assumed to be of the same length.
     */
    void select_max_symbols(const iSaxWord &other);

   private:
    vec<SaxNumBitsT> m_num_bits;
};

#endif  // ISAX_WORD_HPP

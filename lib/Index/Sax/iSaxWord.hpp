#ifndef INDEX_SAX_ISAXWORD_HPP
#define INDEX_SAX_ISAXWORD_HPP

#include <optional>

#include "Index/Sax/SaxWord.hpp"
#include "Util/Types/Numbers.hpp"
#include "Util/Types/Vec.hpp"

/** @brief indexable Symbolic Aggregate approXimation (iSAX) word */
class iSaxWord : public SaxWord {
   public:
    ~iSaxWord() = default;

    /**
     * @brief Constructor from the PAA of a time series
     * @param paa The Piecewise Aggregate Approximation (PAA) of a time series
     * @param breakpoints The breakpoints to use for the symbols
     * @param alphabet_num_bits The number of bits used by the breakpoints
     * @param num_bits The number of bits to use for the symbols
     */
    inline iSaxWord(const vec<Real> &paa, const vec<Real> &breakpoints, const SaxNumBitsT alphabet_num_bits,
                    const vec<SaxNumBitsT> &num_bits)
        : SaxWord(paa, breakpoints, alphabet_num_bits), m_num_bits(num_bits) {
        assert(paa.size() == m_num_bits.size());
        assert(m_alphabet_num_bits >= *std::max_element(m_num_bits.begin(), m_num_bits.end()));
    }

    /**
     * @brief Constructor from the PAA of a time series with the same number of bits for each segment
     * @param paa The Piecewise Aggregate Approximation (PAA) of a time series
     * @param breakpoints The breakpoints to use for the symbols
     * @param alphabet_num_bits The number of bits to used by the breakpoints
     */
    inline iSaxWord(const vec<Real> &paa, const vec<Real> &breakpoints, const SaxNumBitsT alphabet_num_bits)
        : SaxWord(paa, breakpoints, alphabet_num_bits), m_num_bits(paa.size(), alphabet_num_bits) {}

    iSaxWord() = default;

    /**
     * @brief Copy constructor
     * @param other The iSaxWord to copy
     */
    iSaxWord(const iSaxWord &) = default;

    /**
     * @brief Constructor with the same number of bits for each segment
     * @param symbols The symbols of the word
     * @param alphabet_num_bits The number of bits to use for the symbols. All symbols will have the same number of
     * bits.
     */
    iSaxWord(vec<SaxSymbolT> symbols, SaxNumBitsT alphabet_num_bits);

    /**
     * @brief Get the symbol at the given index
     * @param index The index of the symbol
     * @return The symbol at the given index
     */
    inline SaxSymbolT operator[](SaxSegIndT index) const override {
        return static_cast<SaxSymbolT>(m_symbols[index] >> (m_alphabet_num_bits - m_num_bits[index]));
    }

    /**
     * @brief Get the symbol at the given index without shifting; TODO: this should be removed, and the iSAX symbol
     * access logic reworked
     * @param index The index of the symbol
     * @return The symbol at the given index without shifting
     */
    SaxSymbolT symbol_no_shift(SaxSegIndT index) const;

    /**
     * @brief Get the number of segments in the iSAX word
     * @return The number of segments in the iSAX word
     */
    inline SaxSegIndT size() const { return static_cast<SaxSegIndT>(m_symbols.size()); }

    /**
     * @brief Get the number of bits used for the symbol at the given index
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
     * @param seg_ind The index of the symbol to update
     * @return The bit that was appended to the symbol
     */
    uint8_t apply_split(SaxSegIndT seg_ind);

    /**
     * @brief Unsplit a segment; updates the iSAX word inplace
     * @param seg_ind The index of the symbol to update
     */
    void unsplit(SaxSegIndT seg_ind);

    /**
     * @brief Increase the cardinality of a segment and set the new bit
     * @param seg_ind The index of the symbol to update
     * @param bit The new bit
     */
    void set_new_bit(SaxSegIndT seg_ind, uint8_t bit);

    /**
     * @brief Appends a bit to the symbol at the given index; updates the iSAX word inplace
     * @param index The index of the symbol
     * @param bit The bit to append
     */
    void append_to_symbol(SaxSegIndT index, uint8_t bit);

    /**
     * @brief Set the symbol at the given index
     * @param index The index of the symbol
     * @param num_bits Number of bits to use for the symbol
     * @param symbol Symbol (represented using `num_bits` bits)
     */
    void set_symbol(SaxSegIndT index, SaxNumBitsT num_bits, SaxSymbolT symbol);

    /**
     * @brief For each segment, set the symbols to the maximum of two words
     * @param other The other iSaxWord to compare to. Assumed to be of the same length.
     */
    void select_max_symbols(const iSaxWord &other);

    /**
     * @brief Get the breakpoint in the middle of the interval of a given segment
     * @param segment_ind The index of the segment
     * @param breakpoints The breakpoints of the iSAX word
     * @return The breakpoint in the middle of the interval of the given segment
     */
    std::optional<Real> get_mid_breakpoint(SaxSegIndT segment_ind, const vec<Real> &breakpoints) const;

   private:
    vec<SaxNumBitsT> m_num_bits;
};

#endif  // INDEX_SAX_ISAXWORD_HPP

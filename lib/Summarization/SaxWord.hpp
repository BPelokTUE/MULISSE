#ifndef SAX_WORD_HPP
#define SAX_WORD_HPP

#include "Util/typedefs.hpp"

#include <boost/functional/hash.hpp>

#include <functional>

/** @brief Symbolic Aggregate approXimation (SAX) word */
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

    /**
     * @brief Default constructor
     */
    SaxWord() = default;

    /**
     * @brief Copy constructor.
     *
     * @param other The SaxWord to copy.
     */
    SaxWord(const SaxWord&) = default;

    /**
     * @brief Construct a new SaxWord object.
     *
     * @param symbols The symbols of the word.
     * @param num_bits The number of bits to use for the symbols.
     */
    SaxWord(vec<SaxSymbolT> symbols, SaxNumBitsT num_bits);

    /**
     * @brief Get the symbol at the given index.
     *
     * @param index The index of the symbol.
     * @return The symbol at the given index.
     */
    virtual SaxSymbolT operator[](SaxSegIndT index) const;

    /**
     * @brief Equality operator.
     *
     * @param other The other SaxWord to compare to. Assumed to be of the same length.
     * @return `True` if `this` is equal to the `other`.
     */
    bool operator==(const SaxWord& other) const;

    /**
     * @brief Get the number of bits of the alphabet.
     *
     * @return The number of bits
     */
    SaxNumBitsT get_alphabet_num_bits() const;

    /**
     * @brief Get the length of the word.
     *
     * @return The length of the word.
     */
    size_t size() const;

   protected:
    vec<SaxSymbolT> m_symbols;
    SaxNumBitsT m_alphabet_num_bits;
};

#endif  // SAX_WORD_HPP

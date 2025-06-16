#ifndef INDEX_SAX_SAXWORD_HPP
#define INDEX_SAX_SAXWORD_HPP

#include <cassert>

#include "Index/Entry/Paa.hpp"
#include "Util/HelperFuncs/Conversion.hpp"
#include "Util/Types/Containers.hpp"
#include "Util/Types/Numbers.hpp"

/** @brief Symbolic Aggregate approXimation (SAX) word */
class SaxWord {
    friend class SaxSymbolsFactory;
    friend class SaxEnvelope;

   public:
    virtual ~SaxWord() = default;

    /**
     * @brief Construct a new iSaxWord object from the PAA of a time series.
     * @param paa The Piecewise Aggregate Approximation (PAA) of a time series.
     * @param breakpoints The breakpoints to use for the symbols.
     * @param num_bits The number of bits to use for the symbols.
     * @param alphabet_num_bits The number of bits used by the breakpoints. Defaults to 0 indicating that `num_bits ==
     * alphabet_num_bits`.
     */
    inline SaxWord(const vec<Real>& paa, const vec<Real>& breakpoints, SaxNumBitsT num_bits,
                   SaxNumBitsT alphabet_num_bits = 0) {
        assert(num_bits > 0);
        alphabet_num_bits = alphabet_num_bits == 0 ? num_bits : alphabet_num_bits;
        assert(breakpoints.size() == (1 << alphabet_num_bits) - 1);

        m_alphabet_num_bits = num_bits;
        uint paa_len = U(paa.size());
        m_symbols.resize(paa_len);

        SaxNumBitsT shift = alphabet_num_bits - num_bits;
        for (uint i = 0; i < paa_len; ++i) {
            auto it = std::lower_bound(breakpoints.begin(), breakpoints.end(), paa[i]);
            m_symbols[i] = static_cast<SaxSymbolT>((it - breakpoints.begin()) >> shift);
        }
    }

    /**
     * @brief Default constructor
     */
    SaxWord() = default;

    /**
     * @brief Copy constructor.
     * @param other The SaxWord to copy.
     */
    SaxWord(const SaxWord&) = default;

    /**
     * @brief Construct a new SaxWord object.
     * @param symbols The symbols of the word.
     * @param num_bits The number of bits to use for the symbols.
     */
    SaxWord(vec<SaxSymbolT> symbols, SaxNumBitsT num_bits);

    /**
     * @brief Get the symbol at the given index.
     * @param index The index of the symbol.
     * @return The symbol at the given index.
     */
    virtual SaxSymbolT operator[](SaxSegIndT index) const;

    /**
     * @brief Equality operator.
     * @param other The other SaxWord to compare to. Assumed to be of the same length.
     * @return `True` if `this` is equal to the `other`.
     */
    bool operator==(const SaxWord& other) const;

    /**
     * @brief Get the number of bits of the alphabet.
     * @return The number of bits
     */
    SaxNumBitsT get_alphabet_num_bits() const;

    /**
     * @brief Get the length of the word.
     * @return The length of the word.
     */
    size_t size() const;

   protected:
    vec<SaxSymbolT> m_symbols;
    SaxNumBitsT m_alphabet_num_bits;
};

#endif  // INDEX_SAX_SAXWORD_HPP

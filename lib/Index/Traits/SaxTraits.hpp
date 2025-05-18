#ifndef INDEX_TRAITS_SAXTRAITS_HPP
#define INDEX_TRAITS_SAXTRAITS_HPP

#include <cereal/access.hpp>

#include "Index/Sax/iSaxWord.hpp"
#include "Util/Types/Numbers.hpp"

struct EntrySaxSymbol {
    virtual ~EntrySaxSymbol() = default;
};

struct PaaSaxSymbol : EntrySaxSymbol {
    SaxSymbolT m_symbol;

    PaaSaxSymbol(SaxSymbolT symbol) : m_symbol(symbol) {};

    PaaSaxSymbol() = default;

   private:
    friend class cereal::access;

    template <class Archive>
    void serialize(Archive &ar) {
        ar(m_symbol);
    }
};

struct EnvelopeSaxSymbol : EntrySaxSymbol {
    SaxSymbolT m_min_symbol, m_max_symbol;

    EnvelopeSaxSymbol(SaxSymbolT min_symbol, SaxSymbolT max_symbol)
        : m_min_symbol(min_symbol), m_max_symbol(max_symbol) {};

    EnvelopeSaxSymbol() = default;

   private:
    friend class cereal::access;

    template <class Archive>
    void serialize(Archive &ar) {
        ar(m_min_symbol, m_max_symbol);
    }
};

template <typename T>
    requires std::derived_from<T, EntrySaxSymbol>
struct EntryISax {
    virtual ~EntryISax() = default;

    virtual const std::vector<SaxNumBitsT> &get_num_bits() const = 0;

    virtual T symbol_no_shift(SaxSegIndT index) const = 0;

    virtual size_t size() const = 0;
};

struct PaaISax : EntryISax<PaaSaxSymbol> {
    iSaxWord m_isax_word;

    PaaISax(vec<PaaSaxSymbol> paa_sax_symbol, SaxNumBitsT num_bits);
    PaaISax() = default;
    const vec<SaxNumBitsT> &get_num_bits() const override;
    PaaSaxSymbol symbol_no_shift(SaxSegIndT index) const override;
    size_t size() const override;
};

struct EnvelopeISax : EntryISax<EnvelopeSaxSymbol> {
    iSaxWord m_isax_min;
    iSaxWord m_isax_max;

    EnvelopeISax(vec<EnvelopeSaxSymbol> envelope_sax_symbol, SaxNumBitsT num_bits);
    EnvelopeISax() = default;
    const vec<SaxNumBitsT> &get_num_bits() const override;
    EnvelopeSaxSymbol symbol_no_shift(SaxSegIndT index) const override;
    size_t size() const override;
};

template <typename T>
struct SaxTraits;

template <>
struct SaxTraits<struct PaaTag> {
    using iSaxType = PaaISax;
    using SymbolType = PaaSaxSymbol;
};

template <>
struct SaxTraits<struct EnvelopeTag> {
    using iSaxType = EnvelopeISax;
    using SymbolType = EnvelopeSaxSymbol;
};

#endif  // INDEX_TRAITS_SAXTRAITS_HPP

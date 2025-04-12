#ifndef ISAX_FINALIZED_NODE_HPP
#define ISAX_FINALIZED_NODE_HPP

#include <vector>

#include <cereal/types/polymorphic.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/utility.hpp>
#include <cereal/types/vector.hpp>

#include "Util/typedefs.hpp"
#include "Summarization/IndexEntry.hpp"
#include "Summarization/Envelope.hpp"
#include "Summarization/iSaxWord.hpp"
#include "Search/iSax/iSaxNode.hpp"

using std::pair;

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

template <typename FTag>
concept ValidEntryTraitsTag = std::is_same_v<FTag, PaaTag> || std::is_same_v<FTag, EnvelopeTag>;

/**
 * @brief Base class for nodes in a iSaxFinalizedIndex
 *
 * Derived classes of iSaxFinalizedNode contain information for facilitating search in the iSAX index
 *
 * @tparam FTag The SAX traits to use
 */
template <typename FTag>
    requires ValidEntryTraitsTag<FTag>
class iSaxFinalizedNode : public iSaxNode {
   public:
    virtual ~iSaxFinalizedNode() = default;

    /**
     * @brief Get the left and right children of the node, if any
     * @return Pointers (constant raw) to the left and right children
     */
    virtual pair<const iSaxFinalizedNode<FTag> *, const iSaxFinalizedNode<FTag> *> get_children() const = 0;

    /**
     * @brief Get the iSAX max symbols of the children on the split index
     * @param split_num_bits The number of bits of the split segment before the split
     * @param symbol_num_bits The number of bits used to represent the symbols. If the finalized index was created using
     *        iSaxIndex::finalize, this should be the same as number of bits of the alphabet.
     * @return The iSAX max symbols of the children on the split index
     */
    virtual pair<SaxSymbolT, SaxSymbolT> get_children_max_symbols(SaxNumBitsT split_num_bits,
                                                                  SaxNumBitsT symbol_num_bits) const {
        throw std::runtime_error("get_children_max_symbols is only implemented for EnvelopeTag");
    }
};

template <typename FTag>
    requires ValidEntryTraitsTag<FTag>
struct iSaxInternalNodeArgs {
    virtual ~iSaxInternalNodeArgs() = default;

    SaxSplitIndex m_split_ind;
    uptr<iSaxFinalizedNode<FTag>> m_left;
    uptr<iSaxFinalizedNode<FTag>> m_right;

    iSaxInternalNodeArgs(SaxSplitIndex split_ind, uptr<iSaxFinalizedNode<FTag>> left,
                         uptr<iSaxFinalizedNode<FTag>> right)
        : m_split_ind(split_ind), m_left(std::move(left)), m_right(std::move(right)) {}

    iSaxInternalNodeArgs() = default;

   private:
    friend class cereal::access;

    template <class Archive>
    void serialize(Archive &ar) {
        ar(m_split_ind, m_left, m_right);
    }
};

struct iSaxEnvelopeInternalNodeArgs : iSaxInternalNodeArgs<EnvelopeTag> {
    SaxSymbolT m_max_symbol_left;
    SaxSymbolT m_max_symbol_right;

    iSaxEnvelopeInternalNodeArgs(SaxSplitIndex split_ind, SaxSymbolT max_symbol_left, SaxSymbolT max_symbol_right,
                                 uptr<iSaxFinalizedNode<EnvelopeTag>> left, uptr<iSaxFinalizedNode<EnvelopeTag>> right)
        : iSaxInternalNodeArgs(split_ind, std::move(left), std::move(right)),
          m_max_symbol_left(max_symbol_left),
          m_max_symbol_right(max_symbol_right) {}

    iSaxEnvelopeInternalNodeArgs() = default;

   private:
    friend class cereal::access;

    template <class Archive>
    void serialize(Archive &ar) {
        ar(cereal::base_class<iSaxInternalNodeArgs<EnvelopeTag>>(this), m_max_symbol_left, m_max_symbol_right);
    }
};

/**
 * @brief Finalized internal node
 * @tparam FTag The SAX traits to use
 * */
template <typename FTag>
    requires ValidEntryTraitsTag<FTag>
class iSaxFinalizedInternal : public iSaxFinalizedNode<FTag> {
   public:
    iSaxFinalizedInternal() = default;

    /**
     * @brief Constructor
     * @param args Arguments for the internal node, dependent on the SAX traits
     */
    iSaxFinalizedInternal(uptr<iSaxInternalNodeArgs<FTag>> args) : m_args(std::move(args)) {};

    virtual pair<const iSaxFinalizedNode<FTag> *, const iSaxFinalizedNode<FTag> *> get_children() const override {
        return {m_args->m_left.get(), m_args->m_right.get()};
    }

    virtual SaxSplitIndex get_split_ind() const override { return m_args->m_split_ind; }

    virtual vec<SubsequenceInfo> get_subsequence_infos() const override { return {}; }

    virtual bool is_leaf() const override { return false; }

    pair<SaxSymbolT, SaxSymbolT> get_children_max_symbols(SaxNumBitsT split_num_bits,
                                                          SaxNumBitsT symbol_num_bits) const override;

   private:
    uptr<iSaxInternalNodeArgs<FTag>> m_args;

    // Required for Cereal (de)serialization
    friend class cereal::access;

    template <class Archive>
    void serialize(Archive &ar) {
        ar(m_args);
    }
};

/**
 * @brief Finalized leaf node
 * @tparam T The SAX traits to use
 */
template <typename T>
    requires ValidEntryTraitsTag<T>
class iSaxFinalizedLeaf : public iSaxFinalizedNode<T> {
   public:
    iSaxFinalizedLeaf() = default;

    /**
     * @brief Construct a new leaf node with the provided file positions and summaries
     * @param subsequence_positions The position and length of the subsequences in the dataset
     */
    iSaxFinalizedLeaf(vec<SubsequenceInfo> subsequence_positions) : m_subsequence_infos(subsequence_positions) {}

    virtual pair<const iSaxFinalizedNode<T> *, const iSaxFinalizedNode<T> *> get_children() const override {
        return {nullptr, nullptr};
    }

    virtual SaxSplitIndex get_split_ind() const override { return {0, 0}; }

    virtual vec<SubsequenceInfo> get_subsequence_infos() const override { return m_subsequence_infos; }

    virtual bool is_leaf() const override { return true; }

    pair<SaxSymbolT, SaxSymbolT> get_children_max_symbols(SaxNumBitsT split_num_bits,
                                                          SaxNumBitsT symbol_num_bits) const override;

   private:
    vec<SubsequenceInfo> m_subsequence_infos;

    // Required for Cereal (de)serialization
    friend class cereal::access;

    template <class Archive>
    void serialize(Archive &ar) {
        ar(m_subsequence_infos);
    }
};

#endif  // ISAX_FINALIZED_NODE_HPP

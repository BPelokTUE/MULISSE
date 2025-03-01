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
    SaxSymbolT symbol;

    PaaSaxSymbol(SaxSymbolT symbol) : symbol(symbol) {};

    PaaSaxSymbol() = default;

   private:
    friend class cereal::access;

    template <class Archive>
    void serialize(Archive &ar) {
        ar(symbol);
    }
};

struct EnvelopeSaxSymbol : EntrySaxSymbol {
    SaxSymbolT min_symbol, max_symbol;

    EnvelopeSaxSymbol(SaxSymbolT min_symbol, SaxSymbolT max_symbol) : min_symbol(min_symbol), max_symbol(max_symbol) {};

    EnvelopeSaxSymbol() = default;

   private:
    friend class cereal::access;

    template <class Archive>
    void serialize(Archive &ar) {
        ar(min_symbol, max_symbol);
    }
};

template <typename T>
    requires std::derived_from<T, EntrySaxSymbol>
struct EntryISax {
    virtual ~EntryISax() = default;

    virtual const std::vector<SaxNumBitsT> &get_num_bits() const = 0;

    virtual T symbol_no_shift(SaxSegIndT index) const = 0;
};

struct PaaISax : EntryISax<PaaSaxSymbol> {
    iSaxWord isax_word;

    PaaISax(vec<PaaSaxSymbol> paa_sax_symbol, SaxNumBitsT num_bits);
    PaaISax() = default;
    const vec<SaxNumBitsT> &get_num_bits() const override;
    PaaSaxSymbol symbol_no_shift(SaxSegIndT index) const override;
};

struct EnvelopeISax : EntryISax<EnvelopeSaxSymbol> {
    iSaxWord isax_min;
    iSaxWord isax_max;

    EnvelopeISax(vec<EnvelopeSaxSymbol> envelope_sax_symbol, SaxNumBitsT num_bits);
    EnvelopeISax() = default;
    const vec<SaxNumBitsT> &get_num_bits() const override;
    EnvelopeSaxSymbol symbol_no_shift(SaxSegIndT index) const override;
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
concept ValidSaxTraitsTag = std::is_same_v<FTag, PaaTag> || std::is_same_v<FTag, EnvelopeTag>;

/**
 * @brief Base class for nodes in a iSaxFinalizedIndex
 *
 * Derived classes of iSaxFinalizedNode contain information for facilitating search in the iSAX index
 *
 * @tparam FTag The SAX traits to use
 */
template <typename FTag>
    requires ValidSaxTraitsTag<FTag>
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
    requires ValidSaxTraitsTag<FTag>
struct iSaxInternalNodeArgs {
    SaxSplitIndex split_ind;
    uptr<iSaxFinalizedNode<FTag>> left;
    uptr<iSaxFinalizedNode<FTag>> right;

    iSaxInternalNodeArgs(SaxSplitIndex split_ind, uptr<iSaxFinalizedNode<FTag>> left,
                         uptr<iSaxFinalizedNode<FTag>> right)
        : split_ind(split_ind), left(std::move(left)), right(std::move(right)) {}

    iSaxInternalNodeArgs() = default;

   private:
    friend class cereal::access;

    template <class Archive>
    void serialize(Archive &ar) {
        ar(split_ind, left, right);
    }
};

struct iSaxEnvelopeInternalNodeArgs : iSaxInternalNodeArgs<EnvelopeTag> {
    SaxSymbolT max_symbol_left;
    SaxSymbolT max_symbol_right;

    iSaxEnvelopeInternalNodeArgs(SaxSplitIndex split_ind, SaxSymbolT max_symbol_left, SaxSymbolT max_symbol_right,
                                 uptr<iSaxFinalizedNode<EnvelopeTag>> left, uptr<iSaxFinalizedNode<EnvelopeTag>> right)
        : iSaxInternalNodeArgs(split_ind, std::move(left), std::move(right)),
          max_symbol_left(max_symbol_left),
          max_symbol_right(max_symbol_right) {}

    iSaxEnvelopeInternalNodeArgs() = default;

   private:
    friend class cereal::access;

    template <class Archive>
    void serialize(Archive &ar) {
        ar(cereal::base_class<iSaxInternalNodeArgs<EnvelopeTag>>(this), max_symbol_left, max_symbol_right);
    }
};

/**
 * @brief Finalized internal node
 * @tparam FTag The SAX traits to use
 * */
template <typename FTag>
    requires ValidSaxTraitsTag<FTag>
class iSaxFinalizedInternal : public iSaxFinalizedNode<FTag> {
   public:
    iSaxFinalizedInternal() = default;

    /**
     * @brief Constructor
     * @param args Arguments for the internal node, dependent on the SAX traits
     */
    iSaxFinalizedInternal(uptr<iSaxInternalNodeArgs<FTag>> args) : m_args(std::move(args)) {};

    virtual pair<const iSaxFinalizedNode<FTag> *, const iSaxFinalizedNode<FTag> *> get_children() const override {
        return {m_args->left.get(), m_args->right.get()};
    }

    virtual SaxSplitIndex get_split_ind() const override { return m_args->split_ind; }

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

// Required for Cereal (de)serialization
CEREAL_REGISTER_TYPE(iSaxFinalizedInternal<EnvelopeTag>)
CEREAL_REGISTER_POLYMORPHIC_RELATION(iSaxFinalizedNode<EnvelopeTag>, iSaxFinalizedInternal<EnvelopeTag>)

/**
 * @brief Finalized leaf node
 * @tparam T The SAX traits to use
 */
template <typename T>
    requires ValidSaxTraitsTag<T>
class iSaxFinalizedLeaf : public iSaxFinalizedNode<T> {
   public:
    iSaxFinalizedLeaf() = default;

    /**
     * @brief Construct a new leaf node with the provided file positions and summaries
     * @param subsequence_positions The position and length of the subsequences in the dataset
     */
    iSaxFinalizedLeaf(vec<SubsequenceInfo> subsequence_positions) : m_subsequence_positions(subsequence_positions) {}

    virtual pair<const iSaxFinalizedNode<T> *, const iSaxFinalizedNode<T> *> get_children() const override {
        return {nullptr, nullptr};
    }

    virtual SaxSplitIndex get_split_ind() const override { return {0, 0}; }

    virtual vec<SubsequenceInfo> get_subsequence_infos() const override { return m_subsequence_positions; }

    virtual bool is_leaf() const override { return true; }

    pair<SaxSymbolT, SaxSymbolT> get_children_max_symbols(SaxNumBitsT split_num_bits,
                                                          SaxNumBitsT symbol_num_bits) const override;

   private:
    vec<SubsequenceInfo> m_subsequence_positions;

    // Required for Cereal (de)serialization
    friend class cereal::access;

    template <class Archive>
    void serialize(Archive &ar) {
        ar(m_subsequence_positions);
    }
};

// Required for Cereal (de)serialization
CEREAL_REGISTER_TYPE(iSaxFinalizedLeaf<EnvelopeTag>)
CEREAL_REGISTER_POLYMORPHIC_RELATION(iSaxFinalizedNode<EnvelopeTag>, iSaxFinalizedLeaf<EnvelopeTag>)

#endif  // ISAX_FINALIZED_NODE_HPP

#ifndef ISAX_FINALIZED_NODE_HPP
#define ISAX_FINALIZED_NODE_HPP

#include <vector>

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

template <typename T>
concept ValidSaxTraitsTag = std::is_same_v<T, PaaTag> || std::is_same_v<T, EnvelopeTag>;

/**
 * @brief Base class for nodes in a iSaxFinalizedIndex
 *
 * Derived classes of iSaxFinalizedNode contain information for facilitating search in the iSAX index
 *
 * @tparam T The SAX traits to use
 */
template <typename T>
    requires ValidSaxTraitsTag<T>
class iSaxFinalizedNode : public iSaxNode {
   public:
    virtual ~iSaxFinalizedNode() = default;

    /**
     * @brief Get the left and right children of the node, if any
     * @return Pointers (constant raw) to the left and right children
     */
    virtual pair<const iSaxFinalizedNode<T> *, const iSaxFinalizedNode<T> *> get_children() const = 0;
};

class iSaxEnvelopeFinalizedNode : public iSaxFinalizedNode<EnvelopeTag> {
   public:
    /**
     * @brief Get the iSAX max symbols of the children on the split index
     * @param split_num_bits The number of bits of the split segment before the split
     * @param symbol_num_bits The number of bits used to represent the symbols. If the finalized index was created using
     *        iSaxIndex::finalize, this should be the same as number of bits of the alphabet.
     * @return The iSAX max symbols of the children on the split index
     */
    virtual pair<SaxSymbolT, SaxSymbolT> get_children_max_symbols(SaxNumBitsT split_num_bits,
                                                                  SaxNumBitsT symbol_num_bits) const = 0;
};

/**
 * @brief Finalized internal node
 * @tparam T The SAX traits to use
 * */
template <typename T>
    requires ValidSaxTraitsTag<T>
class iSaxFinalizedInternal : public iSaxFinalizedNode<T> {
   public:
    iSaxFinalizedInternal() = default;

    /**
     * @brief Constructor
     * @param split_ind Segment and channel index to split on (see SaxSplitIndex)
     * @param left Unique pointer to the left child
     * @param right Unique pointer to the right child
     */
    iSaxFinalizedInternal(SaxSplitIndex split_ind, uptr<iSaxFinalizedNode<T>> left, uptr<iSaxFinalizedNode<T>> right)
        : m_split_ind(split_ind), m_left(std::move(left)), m_right(std::move(right)) {}

    virtual pair<const iSaxFinalizedNode<T> *, const iSaxFinalizedNode<T> *> get_children() const override {
        return {m_left.get(), m_right.get()};
    }

    virtual SaxSplitIndex get_split_ind() const override { return m_split_ind; }

    virtual vec<SubsequencePosition> get_subsequence_positions() const override { return {}; }

    virtual bool is_leaf() const override { return false; }

   protected:
    SaxSplitIndex m_split_ind;
    uptr<iSaxFinalizedNode<T>> m_left = nullptr, m_right = nullptr;

   private:
    // Required for Cereal (de)serialization
    friend class cereal::access;

    template <class Archive>
    void serialize(Archive &ar) {
        ar(m_split_ind, m_left, m_right);
    }
};

class iSaxEnvelopeFinalizedInternal : public iSaxEnvelopeFinalizedNode, public iSaxFinalizedInternal<EnvelopeTag> {
   public:
    /**
     * @brief Constructor
     * @param split_ind Segment and channel index to split on (see SaxSplitIndex)
     * @param isax_max_left iSAX max symbol of the left child in `split_ind`
     * @param isax_max_right iSAX max symbol of the right child in `split_ind`
     * @param left Unique pointer to the left child
     * @param right Unique pointer to the right child
     */
    iSaxEnvelopeFinalizedInternal(SaxSplitIndex split_ind, SaxSymbolT max_symbol_left, SaxSymbolT max_symbol_right,
                                  uptr<iSaxFinalizedNode<EnvelopeTag>> left, uptr<iSaxFinalizedNode<EnvelopeTag>> right)
        : iSaxFinalizedInternal<EnvelopeTag>(split_ind, std::move(left), std::move(right)),
          m_max_symbol_left(max_symbol_left),
          m_max_symbol_right(max_symbol_right) {}

    iSaxEnvelopeFinalizedInternal() = default;

    pair<SaxSymbolT, SaxSymbolT> get_children_max_symbols(SaxNumBitsT split_num_bits,
                                                          SaxNumBitsT symbol_num_bits) const override {
        SaxNumBitsT shift = symbol_num_bits - split_num_bits - 1;
        assert(shift >= 0);
        return {m_max_symbol_left >> shift, m_max_symbol_right >> shift};
    }

    pair<const iSaxFinalizedNode<EnvelopeTag> *, const iSaxFinalizedNode<EnvelopeTag> *> get_children() const override {
        return iSaxFinalizedInternal<EnvelopeTag>::get_children();
    }

    SaxSplitIndex get_split_ind() const override { return iSaxFinalizedInternal<EnvelopeTag>::get_split_ind(); }

    vec<SubsequencePosition> get_subsequence_positions() const override {
        return iSaxFinalizedInternal<EnvelopeTag>::get_subsequence_positions();
    }

    bool is_leaf() const override { return iSaxFinalizedInternal<EnvelopeTag>::is_leaf(); }

   private:
    SaxSymbolT m_max_symbol_left, m_max_symbol_right;

   private:
    // Required for Cereal (de)serialization
    friend class cereal::access;

    template <class Archive>
    void serialize(Archive &ar) {
        ar(m_split_ind, m_max_symbol_left, m_max_symbol_right, m_left, m_right);
    }
};

// Required for Cereal (de)serialization
CEREAL_REGISTER_TYPE(iSaxEnvelopeFinalizedInternal)
CEREAL_REGISTER_POLYMORPHIC_RELATION(iSaxFinalizedInternal<EnvelopeTag>, iSaxEnvelopeFinalizedInternal)
CEREAL_REGISTER_POLYMORPHIC_RELATION(iSaxEnvelopeFinalizedNode, iSaxEnvelopeFinalizedInternal)

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
     * @param subsequence_positions The position of th subsequence in the dataset
     */
    iSaxFinalizedLeaf(vec<SubsequencePosition> subsequence_positions)
        : m_subsequence_positions(subsequence_positions) {}

    virtual pair<const iSaxFinalizedNode<T> *, const iSaxFinalizedNode<T> *> get_children() const override {
        return {nullptr, nullptr};
    }

    virtual SaxSplitIndex get_split_ind() const override { return {0, 0}; }

    virtual vec<SubsequencePosition> get_subsequence_positions() const override { return m_subsequence_positions; }

    virtual bool is_leaf() const override { return true; }

   protected:
    vec<SubsequencePosition> m_subsequence_positions;

   private:
    // Required for Cereal (de)serialization
    friend class cereal::access;

    template <class Archive>
    void serialize(Archive &ar) {
        ar(m_subsequence_positions);
    }
};

class iSaxEnvelopeFinalizedLeaf : public iSaxEnvelopeFinalizedNode, public iSaxFinalizedLeaf<EnvelopeTag> {
   public:
    /**
     * @brief Construct a new leaf node with the provided file positions and envelopes
     * @param subsequence_positions The position of th subsequence in the dataset
     */
    iSaxEnvelopeFinalizedLeaf(vec<SubsequencePosition> subsequence_positions)
        : iSaxFinalizedLeaf<EnvelopeTag>(subsequence_positions) {}

    iSaxEnvelopeFinalizedLeaf() = default;

    pair<SaxSymbolT, SaxSymbolT> get_children_max_symbols(SaxNumBitsT split_num_bits,
                                                          SaxNumBitsT symbol_num_bits) const override {
        return {-1, -1};
    }

    pair<const iSaxFinalizedNode<EnvelopeTag> *, const iSaxFinalizedNode<EnvelopeTag> *> get_children() const override {
        return iSaxFinalizedLeaf<EnvelopeTag>::get_children();
    }

    SaxSplitIndex get_split_ind() const override { return iSaxFinalizedLeaf<EnvelopeTag>::get_split_ind(); }

    vec<SubsequencePosition> get_subsequence_positions() const override {
        return iSaxFinalizedLeaf<EnvelopeTag>::get_subsequence_positions();
    }

    bool is_leaf() const override { return iSaxFinalizedLeaf<EnvelopeTag>::is_leaf(); }
};

// Required for Cereal (de)serialization
CEREAL_REGISTER_TYPE(iSaxEnvelopeFinalizedLeaf)
CEREAL_REGISTER_POLYMORPHIC_RELATION(iSaxFinalizedLeaf<EnvelopeTag>, iSaxEnvelopeFinalizedLeaf)
CEREAL_REGISTER_POLYMORPHIC_RELATION(iSaxEnvelopeFinalizedNode, iSaxEnvelopeFinalizedLeaf)

#endif  // ISAX_FINALIZED_NODE_HPP

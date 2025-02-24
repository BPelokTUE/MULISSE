#ifndef ISAX_FINALIZED_NODE_HPP
#define ISAX_FINALIZED_NODE_HPP

#include <vector>

#include <cereal/types/memory.hpp>
#include <cereal/types/utility.hpp>
#include <cereal/types/vector.hpp>

#include "Util/typedefs.hpp"
#include "Summarization/IndexEntry.hpp"
#include "Summarization/Envelope.hpp"
#include "Search/iSax/iSaxNode.hpp"

using std::pair;

/**
 * @brief Base class for nodes in a iSaxFinalizedIndex
 *
 * Derived classes of iSaxFinalizedNode contain information for facilitating search in the iSAX index
 */
template <typename T>
    requires DerivedFromEntryData<T>
class iSaxFinalizedNode : public iSaxNode {
   public:
    virtual ~iSaxFinalizedNode() = default;

    /**
     * @brief Get the left and right children of the node, if any
     *
     * @return Pointers (constant raw) to the left and right children
     */
    virtual pair<const iSaxFinalizedNode<T> *, const iSaxFinalizedNode<T> *> get_children() const = 0;
};

class iSaxEnvelopeFinalizedNode {
   public:
    /**
     * @brief Get the iSAX max symbols of the children on the split index
     *
     * @param split_num_bits The number of bits of the split segment before the split
     * @param symbol_num_bits The number of bits used to represent the symbols. If the finalized index was created using
     *        iSaxIndex::finalize, this should be the same as number of bits of the alphabet.
     * @return The iSAX max symbols of the children on the split index
     */
    virtual pair<SaxSymbolT, SaxSymbolT> get_children_max_symbols(SaxNumBitsT split_num_bits,
                                                                  SaxNumBitsT symbol_num_bits) const = 0;
};

/** @brief Finalized internal node */
template <typename T>
    requires DerivedFromEntryData<T>
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

    pair<const iSaxFinalizedNode<T> *, const iSaxFinalizedNode<T> *> get_children() const override {
        return {m_left.get(), m_right.get()};
    }

    SaxSplitIndex get_split_ind() const override { return m_split_ind; }

    vec<SubsequencePosition> get_subsequence_positions() const override { return {}; }

    bool is_leaf() const override { return false; }

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

class iSaxEnvelopeFinalizedInternal : public iSaxFinalizedInternal<Envelope>, public iSaxEnvelopeFinalizedNode {
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
                                  uptr<iSaxFinalizedNode<Envelope>> left, uptr<iSaxFinalizedNode<Envelope>> right)
        : iSaxFinalizedInternal<Envelope>(split_ind, std::move(left), std::move(right)),
          m_max_symbol_left(max_symbol_left),
          m_max_symbol_right(max_symbol_right) {}

    pair<SaxSymbolT, SaxSymbolT> get_children_max_symbols(SaxNumBitsT split_num_bits,
                                                          SaxNumBitsT symbol_num_bits) const override {
        SaxNumBitsT shift = symbol_num_bits - split_num_bits - 1;
        assert(shift >= 0);
        return {m_max_symbol_left >> shift, m_max_symbol_right >> shift};
    }

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
CEREAL_REGISTER_POLYMORPHIC_RELATION(iSaxFinalizedInternal<Envelope>, iSaxEnvelopeFinalizedInternal)
CEREAL_REGISTER_POLYMORPHIC_RELATION(iSaxEnvelopeFinalizedNode, iSaxEnvelopeFinalizedInternal)

/**
 * @brief Finalized leaf node
 * @tparam T The type of data stored in the index
 */
template <typename T>
    requires DerivedFromEntryData<T>
class iSaxFinalizedLeaf : public iSaxFinalizedNode<T> {
   public:
    iSaxFinalizedLeaf() = default;

    /**
     * @brief Construct a new leaf node with the provided file positions and summaries
     * @param subsequence_positions The position of th subsequence in the dataset
     */
    iSaxFinalizedLeaf(vec<SubsequencePosition> subsequence_positions)
        : m_subsequence_positions(subsequence_positions) {}

    pair<const iSaxFinalizedNode<T> *, const iSaxFinalizedNode<T> *> get_children() const override {
        return {nullptr, nullptr};
    }

    SaxSplitIndex get_split_ind() const override { return {0, 0}; }

    vec<SubsequencePosition> get_subsequence_positions() const override { return m_subsequence_positions; }

    bool is_leaf() const override { return true; }

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

class iSaxEnvelopeFinalizedLeaf : public iSaxFinalizedLeaf<Envelope>, public iSaxEnvelopeFinalizedNode {
   public:
    /**
     * @brief Construct a new leaf node with the provided file positions and envelopes
     * @param subsequence_positions The position of th subsequence in the dataset
     */
    iSaxEnvelopeFinalizedLeaf(vec<SubsequencePosition> subsequence_positions)
        : iSaxFinalizedLeaf<Envelope>(subsequence_positions) {}

    pair<SaxSymbolT, SaxSymbolT> get_children_max_symbols(SaxNumBitsT split_num_bits,
                                                          SaxNumBitsT symbol_num_bits) const override {
        return {-1, -1};
    }
};

// Required for Cereal (de)serialization
CEREAL_REGISTER_TYPE(iSaxEnvelopeFinalizedLeaf)
CEREAL_REGISTER_POLYMORPHIC_RELATION(iSaxFinalizedLeaf<Envelope>, iSaxEnvelopeFinalizedLeaf)
CEREAL_REGISTER_POLYMORPHIC_RELATION(iSaxEnvelopeFinalizedNode, iSaxEnvelopeFinalizedLeaf)

#endif  // ISAX_FINALIZED_NODE_HPP

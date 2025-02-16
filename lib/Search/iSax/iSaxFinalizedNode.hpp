#ifndef ISAX_FINALIZED_NODE_HPP
#define ISAX_FINALIZED_NODE_HPP

#include <vector>

#include <cereal/types/memory.hpp>
#include <cereal/types/utility.hpp>
#include <cereal/types/vector.hpp>

#include "Util/typedefs.hpp"
#include "Search/iSax/iSaxNode.hpp"

using std::pair;

/**
 * @brief Base class for nodes in a iSaxEnvelopeFinalizedIndex
 *
 * Derived classes of iSaxFinalizedNode contain information for facilitating search in the iSAX index
 */
class iSaxFinalizedNode : public iSaxNode {
   public:
    virtual ~iSaxFinalizedNode() = default;

    /**
     * @brief Get the left and right children of the node, if any
     *
     * @return Pointers (constant raw) to the left and right children
     */
    virtual pair<const iSaxFinalizedNode *, const iSaxFinalizedNode *> get_children() const = 0;

    /**
     * @brief Get the iSAX max symbols of the children on the split index
     *
     * @param split_num_bits The number of bits of the split segment before the split
     * @param symbol_num_bits The number of bits used to represent the symbols. If the finalized index was created using
     *        iSaxEnvelopeIndex::finalize, this should be the same as number of bits of the alphabet.
     * @return The iSAX max symbols of the children on the split index
     */
    virtual pair<SaxSymbolT, SaxSymbolT> get_children_max_symbols(SaxNumBitsT split_num_bits,
                                                                  SaxNumBitsT symbol_num_bits) const = 0;
};

/** @brief Finalized internal node */
class iSaxFinalizedInternal : public iSaxFinalizedNode {
   public:
    iSaxFinalizedInternal() = default;

    /**
     * @brief Constructor
     *
     * @param split_ind Segment and channel index to split on (see SaxSplitIndex)
     * @param isax_max_left iSAX max symbol of the left child in `split_ind`
     * @param isax_max_right iSAX max symbol of the right child in `split_ind`
     * @param left Unique pointer to the left child
     * @param right Unique pointer to the right child
     */
    iSaxFinalizedInternal(SaxSplitIndex split_ind, SaxSymbolT isax_max_left, SaxSymbolT isax_min_right,
                          uptr<iSaxFinalizedNode> left, uptr<iSaxFinalizedNode> right);

    pair<const iSaxFinalizedNode *, const iSaxFinalizedNode *> get_children() const override;

    pair<SaxSymbolT, SaxSymbolT> get_children_max_symbols(SaxNumBitsT split_num_bits,
                                                          SaxNumBitsT symbol_num_bits) const override;

    SaxSplitIndex get_split_ind() const override;

    vec<SubsequencePosition> get_subsequence_positions() const override;

    bool is_leaf() const override;

   private:
    SaxSplitIndex m_split_ind;
    SaxSymbolT m_max_symbol_left, m_max_symbol_right;
    uptr<iSaxFinalizedNode> m_left = nullptr, m_right = nullptr;

    // Required for Cereal (de)serialization
    friend class cereal::access;

    template <class Archive>
    void serialize(Archive &ar) {
        ar(m_split_ind, m_max_symbol_left, m_max_symbol_right, m_left, m_right);
    }
};

// Required for Cereal (de)serialization
CEREAL_REGISTER_TYPE(iSaxFinalizedInternal)
CEREAL_REGISTER_POLYMORPHIC_RELATION(iSaxFinalizedNode, iSaxFinalizedInternal)

/** @brief Finalized leaf node */
class iSaxFinalizedLeaf : public iSaxFinalizedNode {
   public:
    iSaxFinalizedLeaf() = default;

    /**
     * @brief Construct a new leaf node with the provided file positions and envelopes
     *
     * @param subsequence_positions The position of th subsequence in the dataset
     */
    iSaxFinalizedLeaf(vec<SubsequencePosition> subsequence_positions);

    pair<const iSaxFinalizedNode *, const iSaxFinalizedNode *> get_children() const override;

    pair<SaxSymbolT, SaxSymbolT> get_children_max_symbols(SaxNumBitsT split_num_bits,
                                                          SaxNumBitsT symbol_num_bits) const override;

    SaxSplitIndex get_split_ind() const override;

    vec<SubsequencePosition> get_subsequence_positions() const override;

    bool is_leaf() const override;

   private:
    vec<SubsequencePosition> m_subsequence_positions;

    // Required for Cereal (de)serialization
    friend class cereal::access;

    template <class Archive>
    void serialize(Archive &ar) {
        ar(m_subsequence_positions);
    }
};

// Required for Cereal (de)serialization
CEREAL_REGISTER_TYPE(iSaxFinalizedLeaf)
CEREAL_REGISTER_POLYMORPHIC_RELATION(iSaxFinalizedNode, iSaxFinalizedLeaf)

#endif  // ISAX_FINALIZED_NODE_HPP

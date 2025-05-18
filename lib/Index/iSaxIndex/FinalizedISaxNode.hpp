#ifndef ISAX_FINALIZED_NODE_HPP
#define ISAX_FINALIZED_NODE_HPP

#include <cereal/access.hpp>
#include <cereal/types/base_class.hpp>
#include <vector>

#include "Index/Traits/EntryTags.hpp"
#include "Index/iSaxIndex/iSaxNode.hpp"
#include "Util/Types/Numbers.hpp"
#include "Util/Types/Pointers.hpp"
#include "Util/Types/SaxSplitIndex.hpp"

using std::pair;

/**
 * @brief Base class for nodes in a FinalizedISaxIndex
 *
 * Derived classes of FinalizedISaxNode contain information for facilitating search in the iSAX index
 *
 * @tparam FTag The SAX traits to use
 */
template <typename FTag>
    requires ValidEntryTraitsTag<FTag>
class FinalizedISaxNode : public iSaxNode {
   public:
    virtual ~FinalizedISaxNode() = default;

    /**
     * @brief Get the left and right children of the node, if any
     * @return Pointers (constant raw) to the left and right children
     */
    virtual pair<const FinalizedISaxNode<FTag> *, const FinalizedISaxNode<FTag> *> get_children() const = 0;

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
    uptr<FinalizedISaxNode<FTag>> m_left;
    uptr<FinalizedISaxNode<FTag>> m_right;

    iSaxInternalNodeArgs(SaxSplitIndex split_ind, uptr<FinalizedISaxNode<FTag>> left,
                         uptr<FinalizedISaxNode<FTag>> right)
        : m_split_ind(split_ind), m_left(std::move(left)), m_right(std::move(right)) {}

    iSaxInternalNodeArgs() = default;

   private:
    // Required for Cereal (de)serialization
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
                                 uptr<FinalizedISaxNode<EnvelopeTag>> left, uptr<FinalizedISaxNode<EnvelopeTag>> right)
        : iSaxInternalNodeArgs(split_ind, std::move(left), std::move(right)),
          m_max_symbol_left(max_symbol_left),
          m_max_symbol_right(max_symbol_right) {}

    iSaxEnvelopeInternalNodeArgs() = default;

   private:
    // Required for Cereal (de)serialization
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
class FinalizedISaxInternal : public FinalizedISaxNode<FTag> {
   public:
    FinalizedISaxInternal() = default;

    /**
     * @brief Constructor
     * @param args Arguments for the internal node, dependent on the SAX traits
     */
    FinalizedISaxInternal(uptr<iSaxInternalNodeArgs<FTag>> args) : m_args(std::move(args)) {};

    virtual pair<const FinalizedISaxNode<FTag> *, const FinalizedISaxNode<FTag> *> get_children() const override {
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
class FinalizedISaxLeaf : public FinalizedISaxNode<T> {
   public:
    FinalizedISaxLeaf() = default;

    /**
     * @brief Construct a new leaf node with the provided file positions and summaries
     * @param subsequence_positions The position and length of the subsequences in the dataset
     */
    FinalizedISaxLeaf(vec<SubsequenceInfo> subsequence_positions) : m_subsequence_infos(subsequence_positions) {}

    virtual pair<const FinalizedISaxNode<T> *, const FinalizedISaxNode<T> *> get_children() const override {
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

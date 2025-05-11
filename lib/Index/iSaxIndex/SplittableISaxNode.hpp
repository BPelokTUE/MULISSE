#ifndef INDEX_ISAXINDEX_ISAXSPLITTABLENODE_HPP
#define INDEX_ISAXINDEX_ISAXSPLITTABLENODE_HPP

#include "Util/Types/Pointers.hpp"
#include "Index/iSaxIndex/FinalizedISaxNode.hpp"
#include "Index/Entry/EntryData.hpp"
#include "Index/Entry/Paa.hpp"
#include "Index/Entry/Envelope.hpp"

struct FinalizationResult {
    virtual ~FinalizationResult() = default;
};

struct PaaFinalizationResult : public FinalizationResult {
    uptr<FinalizedISaxNode<PaaTag>> m_finalized_node;

    PaaFinalizationResult(uptr<FinalizedISaxNode<PaaTag>> finalized_node)
        : m_finalized_node(std::move(finalized_node)) {}
};

struct EnvelopeFinalizationResult : public FinalizationResult {
    uptr<FinalizedISaxNode<EnvelopeTag>> m_finalized_node;
    vec<iSaxWord> m_isax_max;

    EnvelopeFinalizationResult(uptr<FinalizedISaxNode<EnvelopeTag>> finalized_node, vec<iSaxWord> isax_max)
        : m_finalized_node(std::move(finalized_node)), m_isax_max(isax_max) {}
};

/**
 * @brief Base class for nodes in a iSaxIndex
 * @tparam T The type of data stored in the index
 * Derived classes of SplittableISaxNode contain information for facilitating insertion into the iSAX index
 * */
template <typename T>
    requires DerivedFromEntryData<T>
class SplittableISaxNode : public iSaxNode {
   public:
    virtual ~SplittableISaxNode() = default;

    /**
     * @brief Get the left and right children of the node, if any
     *
     * @return Pointers (constant raw) to the left and right children
     */
    virtual std::pair<const SplittableISaxNode *, const SplittableISaxNode *> get_children() const = 0;

    /**
     * @brief Get the summaries stored in the node, if any
     *
     * @return Vector of multivariate summaries
     */
    virtual vec<vec<T>> get_summaries() const = 0;

    /**
     * @brief Transform the node into a finalized node
     * @param isax_word_settings The settings for the iSAX word
     * @return A unique pointer to the finalization result
     */
    virtual uptr<FinalizationResult> finalize() = 0;
};

/**
 * @brief Splittable internal node
 * @tparam T The type of data stored in the index
 * */
template <typename T>
    requires DerivedFromEntryData<T>
class iSaxSplittableInternal : public SplittableISaxNode<T> {
    template <typename U>
        requires DerivedFromEntryData<U>
    friend class iSaxIndex;

   public:
    /**
     * @brief Construct a new internal node with the provided split index
     *
     * @param split_ind The channel and segment index to split on
     * */
    iSaxSplittableInternal(SaxSplitIndex split_ind) : m_split_ind(split_ind) {}

    /**
     * @brief Construct a new internal node with the provided split index and children
     *
     * @param split_ind The channel and segment index to split on
     * @param left The left child
     * @param right The right child
     */
    iSaxSplittableInternal(SaxSplitIndex split_ind, SplittableISaxNode<T> *left, SplittableISaxNode<T> *right)
        : m_split_ind(split_ind), m_left(left), m_right(right) {}

    std::pair<const SplittableISaxNode<T> *, const SplittableISaxNode<T> *> get_children() const override {
        return {m_left.get(), m_right.get()};
    }

    SaxSplitIndex get_split_ind() const override { return m_split_ind; }

    vec<SubsequenceInfo> get_subsequence_infos() const override { return {}; }

    bool is_leaf() const override { return false; }

    vec<vec<T>> get_summaries() const override { return {}; }

    uptr<FinalizationResult> finalize() override;

   protected:
    SaxSplitIndex m_split_ind;
    uptr<SplittableISaxNode<T>> m_left = nullptr, m_right = nullptr;

   private:
    SaxSymbolT m_max_symbol_left, m_max_symbol_right;
};

/** @brief Splittable leaf node */
template <typename T>
    requires DerivedFromEntryData<T>
class iSaxSplittableLeaf : public SplittableISaxNode<T> {
    template <typename U>
        requires DerivedFromEntryData<U>
    friend class iSaxIndex;

   public:
    /**
     * @brief Construct a new leaf node with the provided file positions and summaries
     * @param subsequence_positions The position within the dataset and length of the subsequences stored in the leaf
     * @param summaries The summaries of the subsequences stored in the leaf
     */
    iSaxSplittableLeaf(vec<SubsequenceInfo> subsequence_positions, vec<vec<T>> summaries)
        : m_subsequence_infos(subsequence_positions), m_summaries(summaries) {}

    virtual std::pair<const SplittableISaxNode<T> *, const SplittableISaxNode<T> *> get_children() const override {
        return {nullptr, nullptr};
    }

    SaxSplitIndex get_split_ind() const override { return {0, 0}; }

    vec<SubsequenceInfo> get_subsequence_infos() const override { return m_subsequence_infos; }

    bool is_leaf() const override { return true; }

    vec<vec<T>> get_summaries() const override { return m_summaries; }

    uptr<FinalizationResult> finalize() override;

   protected:
    vec<vec<T>> m_summaries;
    vec<SubsequenceInfo> m_subsequence_infos;
};

uptr<FinalizedISaxNode<PaaTag>> get_paa_node_finalization_result(uptr<SplittableISaxNode<Paa>> &node);

std::pair<uptr<FinalizedISaxNode<EnvelopeTag>>, vec<iSaxWord>> get_envelope_node_finalization_result(
    uptr<SplittableISaxNode<Envelope>> &node);

#endif  // INDEX_ISAXINDEX_ISAXSPLITTABLENODE_HPP

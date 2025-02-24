#ifndef ISAX_SPLITTABLE_NODE_HPP
#define ISAX_SPLITTABLE_NODE_HPP

#include "Util/typedefs.hpp"
#include "Util/constants.hpp"
#include "Search/iSax/iSaxNode.hpp"
#include "Search/iSax/iSaxFinalizedNode.hpp"
#include "Summarization/IndexEntry.hpp"
#include "Summarization/iSaxWord.hpp"

/**
 * @brief Base class for nodes in a iSaxIndex
 * @tparam T The type of data stored in the index
 * Derived classes of iSaxSplittableNode contain information for facilitating insertion into the iSAX index
 * */
template <typename T>
    requires DerivedFromEntryData<T>
class iSaxSplittableNode : public iSaxNode {
   public:
    virtual ~iSaxSplittableNode() = default;

    /**
     * @brief Get the left and right children of the node, if any
     *
     * @return Pointers (constant raw) to the left and right children
     */
    virtual std::pair<const iSaxSplittableNode *, const iSaxSplittableNode *> get_children() const = 0;

    /**
     * @brief Get the summaries stored in the node, if any
     *
     * @return Vector of multivariate summaries
     */
    virtual vec<vec<T>> get_summaries() const = 0;
};

class iSaxEnvelopeSplittableNode : public iSaxSplittableNode<Envelope> {
   public:
    /**
     * @brief Transform the node into a finalized node
     * @param isax_word_settings The settings for the iSAX word
     * @return A unique pointer to the finalized node and the iSAX max of the node
     */
    virtual std::pair<uptr<iSaxFinalizedNode<Envelope>>, vec<iSaxWord>> finalize(
        const iSaxWordSettings &isax_word_settings) = 0;
};

/**
 * @brief Splittable internal node
 * @tparam T The type of data stored in the index
 * */
template <typename T>
    requires DerivedFromEntryData<T>
class iSaxSplittableInternal : public iSaxSplittableNode<T> {
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
    iSaxSplittableInternal(SaxSplitIndex split_ind, iSaxSplittableNode<T> *left, iSaxSplittableNode<T> *right)
        : m_split_ind(split_ind), m_left(left), m_right(right) {}

    std::pair<const iSaxSplittableNode<T> *, const iSaxSplittableNode<T> *> get_children() const override {
        return {m_left.get(), m_right.get()};
    }

    SaxSplitIndex get_split_ind() const override { return m_split_ind; }

    vec<SubsequencePosition> get_subsequence_positions() const override { return {}; }

    bool is_leaf() const override { return false; }

    vec<vec<T>> get_summaries() const override { return {}; }

   protected:
    SaxSplitIndex m_split_ind;
    uptr<iSaxSplittableNode<T>> m_left = nullptr, m_right = nullptr;
};

class iSaxEnvelopeSplittableInternal : public iSaxSplittableInternal<Envelope>, public iSaxEnvelopeSplittableNode {
   public:
    iSaxEnvelopeSplittableInternal(SaxSplitIndex split_ind, SaxSymbolT max_symbol_left, SaxSymbolT max_symbol_right,
                                   iSaxSplittableNode<Envelope> *left, iSaxSplittableNode<Envelope> *right);

    std::pair<uptr<iSaxFinalizedNode<Envelope>>, vec<iSaxWord>> finalize(
        const iSaxWordSettings &isax_word_settings) override;

   private:
    SaxSymbolT m_max_symbol_left, m_max_symbol_right;
};

/** @brief Splittable leaf node */
template <typename T>
    requires DerivedFromEntryData<T>
class iSaxSplittableLeaf : public iSaxSplittableNode<T> {
    friend class iSaxIndex;

   public:
    /**
     * @brief Construct a new leaf node with the provided file positions and summaries
     * @param subsequence_positions The positions within the dataset of the subsequences stored in the leaf
     * @param summaries The summaries of the subsequences stored in the leaf
     */
    iSaxSplittableLeaf(vec<SubsequencePosition> subsequence_positions, vec<vec<T>> summaries)
        : m_subsequence_positions(subsequence_positions), m_summaries(summaries) {}

    virtual std::pair<const iSaxSplittableNode<T> *, const iSaxSplittableNode<T> *> get_children() const override {
        return {nullptr, nullptr};
    }

    SaxSplitIndex get_split_ind() const override { return {0, 0}; }

    vec<SubsequencePosition> get_subsequence_positions() const override { return m_subsequence_positions; }

    bool is_leaf() const override { return true; }

    vec<vec<T>> get_summaries() const override { return m_summaries; }

   protected:
    vec<vec<T>> m_summaries;
    vec<SubsequencePosition> m_subsequence_positions;
};

class iSaxEnvelopeSplittableLeaf : public iSaxSplittableLeaf<Envelope>, public iSaxEnvelopeSplittableNode {
   public:
    /**
     * @brief Construct a new leaf node with the provided file positions and summaries
     * @param subsequence_positions The positions within the dataset of the subsequences stored in the leaf
     * @param envelopes The envelopes within the dataset of the subsequences stored in the leaf
     */
    iSaxEnvelopeSplittableLeaf(vec<SubsequencePosition> subsequence_positions, vec<vec<Envelope>> envelopes);

    std::pair<uptr<iSaxFinalizedNode<Envelope>>, vec<iSaxWord>> finalize(
        const iSaxWordSettings &isax_word_settings) override;
};

#endif  // ISAX_SPLITTABLE_NODE_HPP

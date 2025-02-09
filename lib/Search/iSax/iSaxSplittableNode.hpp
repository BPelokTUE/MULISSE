#ifndef ISAX_SPLITTABLE_NODE_HPP
#define ISAX_SPLITTABLE_NODE_HPP

#include "Util/typedefs.hpp"
#include "Search/iSax/iSaxNode.hpp"
#include "Search/iSax/iSaxFinalizedNode.hpp"
#include "Summarization/Envelope.hpp"
#include "Summarization/iSaxWord.hpp"

/**
 * @brief Base class for nodes in a iSaxEnvelopeIndex
 *
 * Derived classes of iSaxSplittableNode contain information for facilitating insertion into the iSAX index
 * */
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
     * @brief Get the envelopes stored in the node, if any
     *
     * @return Vector of multivariate envelopes
     */
    virtual vec<vec<Envelope>> get_envelopes() const = 0;

    /**
     * @brief Transform the node into a finalized node
     *
     * @param isax_word_settings The settings for the iSAX word
     * @return A unique pointer to the finalized node and the iSAX max of the node
     */
    virtual std::pair<uptr<iSaxFinalizedNode>, vec<iSaxWord>> finalize(const iSaxWordSettings &isax_word_settings) = 0;
};

/** @brief Splittable internal node */
class iSaxSplittableInternal : public iSaxSplittableNode {
    friend class iSaxEnvelopeIndex;

   public:
    /**
     * @brief Construct a new internal node with the provided split index
     *
     * @param split_ind The channel and segment index to split on
     * */
    iSaxSplittableInternal(SaxSplitIndT split_ind);

    /**
     * @brief Construct a new internal node with the provided split index and children
     *
     * @param split_ind The channel and segment index to split on
     * @param left The left child
     * @param right The right child
     */
    iSaxSplittableInternal(SaxSplitIndT split_ind, iSaxSplittableNode *left, iSaxSplittableNode *right);

    std::pair<const iSaxSplittableNode *, const iSaxSplittableNode *> get_children() const override;

    SaxSplitIndT get_split_ind() const override;

    vec<FilePositionT> get_file_positions() const override;

    bool is_leaf() const override;

    vec<vec<Envelope>> get_envelopes() const override;

    std::pair<uptr<iSaxFinalizedNode>, vec<iSaxWord>> finalize(const iSaxWordSettings &isax_word_settings) override;

   private:
    SaxSplitIndT m_split_ind;
    uptr<iSaxSplittableNode> m_left = nullptr, m_right = nullptr;
};

/** @brief Splittable leaf node */
class iSaxSplittableLeaf : public iSaxSplittableNode {
    friend class iSaxEnvelopeIndex;

   public:
    /**
     * @brief Construct a new leaf node with the provided file positions and envelopes
     *
     * @param file_positions The file positions of the envelopes. Each file positions corresponds to the start of the
     * data summarized in the envelope in the first channel of the time series file.
     */
    iSaxSplittableLeaf(vec<FilePositionT> file_positions, vec<vec<Envelope>> envelopes);

    virtual std::pair<const iSaxSplittableNode *, const iSaxSplittableNode *> get_children() const override;

    SaxSplitIndT get_split_ind() const override;

    vec<FilePositionT> get_file_positions() const override;

    bool is_leaf() const override;

    vec<vec<Envelope>> get_envelopes() const override;

    std::pair<uptr<iSaxFinalizedNode>, vec<iSaxWord>> finalize(const iSaxWordSettings &isax_word_settings) override;

   private:
    vec<vec<Envelope>> m_envelopes;
    vec<FilePositionT> m_file_positions;
};

#endif  // ISAX_SPLITTABLE_NODE_HPP

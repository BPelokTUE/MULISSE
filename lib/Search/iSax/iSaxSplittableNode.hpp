#ifndef ISAX_SPLITTABLE_NODE_HPP
#define ISAX_SPLITTABLE_NODE_HPP

#include "typedefs.hpp"
#include "Search/iSax/iSaxNode.hpp"
#include "Search/iSax/iSaxFinalizedNode.hpp"
#include "Summarization/Envelope.hpp"
#include "Summarization/iSaxWord.hpp"

class iSaxSplittableNode : public iSaxNode {
   public:
    virtual ~iSaxSplittableNode() = default;
    virtual std::pair<const iSaxSplittableNode *, const iSaxSplittableNode *> get_children() const = 0;
    virtual vec<vec<Envelope>> get_envelopes() const = 0;
    virtual std::pair<std::unique_ptr<iSaxFinalizedNode>, vec<iSaxWord>> finalize(
        const iSaxWordSettings &isax_word_settings) = 0;
};

class iSaxSplittableInternal : public iSaxSplittableNode {
    friend class iSaxEnvelopeIndex;

   public:
    iSaxSplittableInternal(SaxSplitIndT split_ind);

    iSaxSplittableInternal(SaxSplitIndT split_ind, iSaxSplittableNode *left, iSaxSplittableNode *right);

    std::pair<const iSaxSplittableNode *, const iSaxSplittableNode *> get_children() const override;

    SaxSplitIndT get_split_ind() const override;

    vec<FilePositionT> get_file_positions() const override;

    bool is_leaf() const override;

    vec<vec<Envelope>> get_envelopes() const override;

    std::pair<std::unique_ptr<iSaxFinalizedNode>, vec<iSaxWord>> finalize(
        const iSaxWordSettings &isax_word_settings) override;

   private:
    SaxSplitIndT m_split_ind;
    std::unique_ptr<iSaxSplittableNode> m_left = nullptr, m_right = nullptr;
};

class iSaxSplittableLeaf : public iSaxSplittableNode {
    friend class iSaxEnvelopeIndex;

   public:
    iSaxSplittableLeaf(vec<FilePositionT> file_positions, vec<vec<Envelope>> envelopes);

    virtual std::pair<const iSaxSplittableNode *, const iSaxSplittableNode *> get_children() const override;

    SaxSplitIndT get_split_ind() const override;

    vec<FilePositionT> get_file_positions() const override;

    bool is_leaf() const override;

    vec<vec<Envelope>> get_envelopes() const override;

    std::pair<std::unique_ptr<iSaxFinalizedNode>, vec<iSaxWord>> finalize(
        const iSaxWordSettings &isax_word_settings) override;

   private:
    vec<vec<Envelope>> m_envelopes;
    vec<FilePositionT> m_file_positions;
};

#endif  // ISAX_SPLITTABLE_NODE_HPP

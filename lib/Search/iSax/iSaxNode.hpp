#ifndef ISAX_NODE_HPP
#define ISAX_NODE_HPP

#include "typedefs.hpp"

class iSaxNode {
   public:
    virtual ~iSaxNode() = default;
    virtual std::pair<const iSaxNode *, const iSaxNode *> get_children() const = 0;
    virtual vec<FilePositionT> get_file_positions() const = 0;
    virtual vec<UlisseEnvelope> get_envelopes() const = 0;
    virtual SaxSplitIndT get_split_ind() const = 0;
    virtual bool is_leaf() const = 0;
};

class iSaxInternalNode : public iSaxNode {
    friend class iSaxUlisseEnvelopeIndex;

   public:
    iSaxInternalNode(SaxSplitIndT split_ind);

    std::pair<const iSaxNode *, const iSaxNode *> get_children() const override;
    vec<FilePositionT> get_file_positions() const override;
    vec<UlisseEnvelope> get_envelopes() const override;
    SaxSplitIndT get_split_ind() const override;
    bool is_leaf() const override;

   private:
    SaxSplitIndT m_split_ind;
    std::unique_ptr<iSaxNode> left = nullptr, right = nullptr;
};

class iSaxLeaf : public iSaxNode {
    friend class iSaxUlisseEnvelopeIndex;

   public:
    iSaxLeaf(vec<FilePositionT> file_positions);

    std::pair<const iSaxNode *, const iSaxNode *> get_children() const override;
    vec<FilePositionT> get_file_positions() const override;
    vec<UlisseEnvelope> get_envelopes() const override;
    SaxSplitIndT get_split_ind() const override;
    bool is_leaf() const override;

   protected:
    vec<FilePositionT> m_file_positions;
};

class iSaxSplittableLeaf : public iSaxLeaf {
    friend class iSaxUlisseEnvelopeIndex;

   public:
    iSaxSplittableLeaf(vec<FilePositionT> file_positions, vec<UlisseEnvelope> envelopes);

    vec<UlisseEnvelope> get_envelopes() const override;

   private:
    vec<UlisseEnvelope> m_envelopes;
};

#endif  // ISAX_NODE_HPP

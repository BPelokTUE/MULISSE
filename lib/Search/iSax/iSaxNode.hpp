#ifndef ISAX_NODE_HPP
#define ISAX_NODE_HPP

#include "typedefs.hpp"

class iSaxNode {
   public:
    virtual ~iSaxNode() = default;
};

class iSaxInternalNode : iSaxNode {
    friend class iSaxUlisseEnvelopeIndex;

   public:
    iSaxInternalNode(SaxSplitIndT split_ind);

   private:
    SaxSplitIndT m_split_ind;
    std::unique_ptr<iSaxNode> left = nullptr, right = nullptr;
};

class iSaxLeaf : public iSaxNode {
    friend class iSaxUlisseEnvelopeIndex;

   public:
    iSaxLeaf(vec<FilePositionT> file_positions);

   protected:
    vec<FilePositionT> m_file_positions;
};

class iSaxSplittableLeaf : public iSaxLeaf {
    friend class iSaxUlisseEnvelopeIndex;

   public:
    iSaxSplittableLeaf(vec<FilePositionT> file_positions, vec<UlisseEnvelope> envelopes);

   private:
    vec<UlisseEnvelope> m_envelopes;
};

#endif  // ISAX_NODE_HPP

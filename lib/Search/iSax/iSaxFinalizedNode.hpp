#include "Search/iSax/iSaxNode.hpp"

class iSaxFinalizedNode : public iSaxNode {
   public:
    virtual ~iSaxFinalizedNode() = default;
    virtual std::pair<const iSaxFinalizedNode *, const iSaxFinalizedNode *> get_children() const = 0;
};

class iSaxFinalizedInternal : public iSaxFinalizedNode {
   public:
    iSaxFinalizedInternal(SaxSplitIndT split_ind, SaxSymbolT isax_max_left, SaxSymbolT isax_min_right);

    std::pair<const iSaxFinalizedNode *, const iSaxFinalizedNode *> get_children() const override;

    SaxSplitIndT get_split_ind() const override;

    vec<FilePositionT> get_file_positions() const override;

    bool is_leaf() const override;

   private:
    SaxSplitIndT m_split_ind;
    SaxSymbolT m_isax_max_left, m_isax_min_right;
    std::unique_ptr<iSaxFinalizedNode> left = nullptr, right = nullptr;
};

class iSaxFinalizedLeaf : public iSaxFinalizedNode {
   public:
    iSaxFinalizedLeaf(vec<FilePositionT> file_positions);

    std::pair<const iSaxFinalizedNode *, const iSaxFinalizedNode *> get_children() const override;

    SaxSplitIndT get_split_ind() const override;

    vec<FilePositionT> get_file_positions() const override;

    bool is_leaf() const override;

   private:
    vec<FilePositionT> m_file_positions;
};

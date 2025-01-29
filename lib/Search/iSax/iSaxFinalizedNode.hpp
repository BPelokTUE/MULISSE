#ifndef ISAX_FINALIZED_NODE_HPP
#define ISAX_FINALIZED_NODE_HPP

#include <cereal/types/memory.hpp>
#include <cereal/types/utility.hpp>

#include "Search/iSax/iSaxNode.hpp"

class iSaxFinalizedNode : public iSaxNode {
   public:
    virtual ~iSaxFinalizedNode() = default;
    virtual std::pair<const iSaxFinalizedNode *, const iSaxFinalizedNode *> get_children() const = 0;
};

class iSaxFinalizedInternal : public iSaxFinalizedNode {
   public:
    iSaxFinalizedInternal() = default;
    iSaxFinalizedInternal(SaxSplitIndT split_ind, SaxSymbolT isax_max_left, SaxSymbolT isax_min_right,
                          uptr<iSaxFinalizedNode> left, uptr<iSaxFinalizedNode> right);

    std::pair<const iSaxFinalizedNode *, const iSaxFinalizedNode *> get_children() const override;

    SaxSplitIndT get_split_ind() const override;

    vec<FilePositionT> get_file_positions() const override;

    bool is_leaf() const override;

   private:
    SaxSplitIndT m_split_ind;
    SaxSymbolT m_max_symbol_left, m_max_symbol_right;
    std::unique_ptr<iSaxFinalizedNode> m_left = nullptr, m_right = nullptr;

    friend class cereal::access;

    template <class Archive>
    void serialize(Archive &ar) {
        ar(m_split_ind, m_max_symbol_left, m_max_symbol_right, m_left, m_right);
    }
};

CEREAL_REGISTER_TYPE(iSaxFinalizedInternal)
CEREAL_REGISTER_POLYMORPHIC_RELATION(iSaxFinalizedNode, iSaxFinalizedInternal)

class iSaxFinalizedLeaf : public iSaxFinalizedNode {
   public:
    iSaxFinalizedLeaf() = default;
    iSaxFinalizedLeaf(vec<FilePositionT> file_positions);

    std::pair<const iSaxFinalizedNode *, const iSaxFinalizedNode *> get_children() const override;

    SaxSplitIndT get_split_ind() const override;

    vec<FilePositionT> get_file_positions() const override;

    bool is_leaf() const override;

   private:
    vec<FilePositionT> m_file_positions;

    friend class cereal::access;

    template <class Archive>
    void serialize(Archive &ar) {
        ar(m_file_positions);
    }
};

CEREAL_REGISTER_TYPE(iSaxFinalizedLeaf)
CEREAL_REGISTER_POLYMORPHIC_RELATION(iSaxFinalizedNode, iSaxFinalizedLeaf)

#endif  // ISAX_FINALIZED_NODE_HPP

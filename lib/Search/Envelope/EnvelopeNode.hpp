#ifndef ENVELOPE_NODE_HPP
#define ENVELOPE_NODE_HPP

#include "Util/typedefs.hpp"
#include "Summarization/Envelope.hpp"

/** @brief Base class for envelope nodes */
class EnvelopeNode {
   public:
    virtual ~EnvelopeNode() = default;

    /**
     * @brief Check whether the node is a leaf or not
     * @return `true` if the node is a leaf, `false` otherwise
     */
    virtual bool is_leaf() const = 0;

    /**
     * @brief Get the channel envelopes for the node
     * @return The envelope of lower and upper bounds for the node
     */
    virtual const vec<Envelope> &get_envelopes() const = 0;

    /**
     * @brief Get the children of the node if any
     * @return Vector of pointers to the children of the node
     */
    virtual const vec<const EnvelopeNode *> get_children() const = 0;

    /**
     * @brief Get the subsequence informations of the envelopes stored in the node if any
     * @return Vector of subsequence informations
     */
    virtual const vec<SubsequenceInfo> *get_subsequence_infos() const = 0;

   protected:
    vec<Envelope> m_envelopes;
};

/** @brief Class representing envelope internal nodes */
class EnvelopeInternal : public EnvelopeNode {
   public:
    EnvelopeInternal() = default;

    EnvelopeInternal(vec<uptr<EnvelopeNode>> children);

    bool is_leaf() const override;

    const vec<Envelope> &get_envelopes() const override;

    const vec<const EnvelopeNode *> get_children() const override;

    const vec<SubsequenceInfo> *get_subsequence_infos() const override;

   private:
    vec<uptr<EnvelopeNode>> m_children;
};

/** @brief Class representing envelope leaf nodes */
class EnvelopeLeaf : public EnvelopeNode {
   public:
    EnvelopeLeaf() = default;

    EnvelopeLeaf(IndexEntry<Envelope> &envelope_entry);

    EnvelopeLeaf(vec<Envelope> &envelopes, vec<SubsequenceInfo> &subs_infos);

    bool is_leaf() const override;

    const vec<Envelope> &get_envelopes() const override;

    const vec<const EnvelopeNode *> get_children() const override;

    const vec<SubsequenceInfo> *get_subsequence_infos() const override;

   private:
    vec<SubsequenceInfo> m_subs_infos;
};

#endif  // ENVELOPE_NODE_HPP

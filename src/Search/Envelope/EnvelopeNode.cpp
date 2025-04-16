#include "Util/typedefs.hpp"
#include "Search/Envelope/EnvelopeGrouper.hpp"

// EnvelopeInternal

EnvelopeInternal::EnvelopeInternal(vec<uptr<EnvelopeNode>> children) : m_children(std::move(children)) {
    assert(!m_children.empty());

    m_envelopes = m_children[0]->get_envelopes();
    for (size_t i = 1; i < m_children.size(); ++i) {
        auto &child_envelopes = m_children[i]->get_envelopes();
        assert(m_envelopes.size() == child_envelopes.size());
        for (MtsNumChannelsT c = 0; c < m_envelopes.size(); ++c) {
            m_envelopes[c].merge(child_envelopes[c]);
        }
    }
}

bool EnvelopeInternal::is_leaf() const { return false; }

const vec<Envelope> &EnvelopeInternal::get_envelopes() const { return m_envelopes; }

const vec<const EnvelopeNode *> EnvelopeInternal::get_children() const {
    vec<const EnvelopeNode *> children(m_children.size());
    for (size_t i = 0; i < m_children.size(); ++i) children[i] = m_children[i].get();
    return children;
}

const vec<SubsequenceInfo> *EnvelopeInternal::get_subsequence_infos() const { return nullptr; }

// EnvelopeLeaf

EnvelopeLeaf::EnvelopeLeaf(IndexEntry<Envelope> &envelope_entry) {
    m_envelopes = envelope_entry.m_mts_summary;
    m_subs_infos = {envelope_entry.m_subs_info};
}

bool EnvelopeLeaf::is_leaf() const { return true; }

const vec<Envelope> &EnvelopeLeaf::get_envelopes() const { return m_envelopes; }

const vec<const EnvelopeNode *> EnvelopeLeaf::get_children() const { return {}; }

const vec<SubsequenceInfo> *EnvelopeLeaf::get_subsequence_infos() const { return &m_subs_infos; }

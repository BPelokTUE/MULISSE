#include "Index/EnvelopeIndex/Tree/EnvelopeNode.hpp"

#include <algorithm>
#include <cassert>

#include "Index/Entry/Envelope.hpp"
#include "Index/Entry/IndexEntry.hpp"
#include "Util/Types/Pointers.hpp"

EnvelopeNode::~EnvelopeNode() = default;

// EnvelopeInternal

EnvelopeInternal::~EnvelopeInternal() = default;

EnvelopeInternal::EnvelopeInternal() = default;

EnvelopeInternal::EnvelopeInternal(vec<uptr<EnvelopeNode>> &&children) : m_children(std::move(children)) {
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

void EnvelopeInternal::merge_subsequence_infos() {
    for (auto &child : m_children) child->merge_subsequence_infos();
}

// EnvelopeLeaf

EnvelopeLeaf::~EnvelopeLeaf() = default;

EnvelopeLeaf::EnvelopeLeaf() = default;

EnvelopeLeaf::EnvelopeLeaf(IndexEntry<Envelope> &envelope_entry) {
    m_envelopes = envelope_entry.m_mts_summary;
    m_subs_infos = {envelope_entry.m_subs_info};
}

EnvelopeLeaf::EnvelopeLeaf(vec<Envelope> envelopes, vec<SubsequenceInfo> subs_infos) {
    m_envelopes = envelopes;
    m_subs_infos = subs_infos;
}

bool EnvelopeLeaf::is_leaf() const { return true; }

const vec<Envelope> &EnvelopeLeaf::get_envelopes() const { return m_envelopes; }

const vec<const EnvelopeNode *> EnvelopeLeaf::get_children() const { return {}; }

const vec<SubsequenceInfo> *EnvelopeLeaf::get_subsequence_infos() const { return &m_subs_infos; }

void EnvelopeLeaf::merge_subsequence_infos() {
    if (m_subs_infos.empty()) return;

    std::sort(m_subs_infos.begin(), m_subs_infos.end());
    vec<SubsequenceInfo> merged_subs_infos;
    merged_subs_infos.push_back(m_subs_infos[0]);

    for (size_t i = 1; i < m_subs_infos.size(); ++i) {
        if (m_subs_infos[i].m_position.m_series == merged_subs_infos.back().m_position.m_series &&
            m_subs_infos[i].m_position.m_start ==
                merged_subs_infos.back().m_position.m_start + merged_subs_infos.back().m_length) {
            merged_subs_infos.back().m_length += m_subs_infos[i].m_length;
        } else {
            merged_subs_infos.push_back(m_subs_infos[i]);
        }
    }
    m_subs_infos = std::move(merged_subs_infos);
}

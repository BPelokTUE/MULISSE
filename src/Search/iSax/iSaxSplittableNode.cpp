#include <iostream>

#include "Search/iSax/iSaxSplittableNode.hpp"

// iSaxSplittableInternal

iSaxSplittableInternal::iSaxSplittableInternal(SaxSplitIndT split_ind) : m_split_ind(split_ind) {}

iSaxSplittableInternal::iSaxSplittableInternal(SaxSplitIndT split_ind, iSaxSplittableNode *left,
                                               iSaxSplittableNode *right)
    : m_split_ind(split_ind), m_left(left), m_right(right) {}

std::pair<const iSaxSplittableNode *, const iSaxSplittableNode *> iSaxSplittableInternal::get_children() const {
    return {m_left.get(), m_right.get()};
}

SaxSplitIndT iSaxSplittableInternal::get_split_ind() const { return m_split_ind; }

vec<FilePositionT> iSaxSplittableInternal::get_file_positions() const { return {}; }

vec<vec<UlisseEnvelope>> iSaxSplittableInternal::get_envelopes() const { return {}; };

bool iSaxSplittableInternal::is_leaf() const { return false; }

std::pair<std::unique_ptr<iSaxFinalizedNode>, vec<iSaxWord>> iSaxSplittableInternal::finalize(
    const iSaxWordSettings &isax_word_settings) const {
    assert(m_left && m_right);

    auto [finalized_left, isax_max_left] = m_left->finalize(isax_word_settings);
    auto [finalized_right, isax_max_right] = m_right->finalize(isax_word_settings);
    auto [seg_ind, channel_ind] = m_split_ind;

    SaxSymbolT max_symbol_left = isax_max_left[channel_ind][seg_ind];
    SaxSymbolT max_symbol_right = isax_max_right[channel_ind][seg_ind];

    auto finalized = std::make_unique<iSaxFinalizedInternal>(m_split_ind, max_symbol_left, max_symbol_right,
                                                             finalized_left.get(), finalized_right.get());

    for (size_t c = 0; c < isax_max_left.size(); ++c) {
        isax_max_left[c].select_max_symbols(isax_max_right[c]);
    }
    return std::make_pair(std::move(finalized), isax_max_left);
};

// iSaxSplittableLeaf

iSaxSplittableLeaf::iSaxSplittableLeaf(vec<FilePositionT> file_positions, vec<vec<UlisseEnvelope>> envelopes)
    : m_file_positions(file_positions), m_envelopes(envelopes) {}

std::pair<const iSaxSplittableNode *, const iSaxSplittableNode *> iSaxSplittableLeaf::get_children() const {
    return {nullptr, nullptr};
};

SaxSplitIndT iSaxSplittableLeaf::get_split_ind() const { return {-1, -1}; }

vec<FilePositionT> iSaxSplittableLeaf::get_file_positions() const { return m_file_positions; }

bool iSaxSplittableLeaf::is_leaf() const { return true; }

vec<vec<UlisseEnvelope>> iSaxSplittableLeaf::get_envelopes() const { return m_envelopes; }

std::pair<std::unique_ptr<iSaxFinalizedNode>, vec<iSaxWord>> iSaxSplittableLeaf::finalize(
    const iSaxWordSettings &isax_word_settings) const {
    assert(m_envelopes.size() > 0);
    assert(m_envelopes[0].size() > 0);

    size_t num_envelopes = m_envelopes.size(), num_channels = m_envelopes[0].size(),
           num_segments = m_envelopes[0][0].second.size();

    vec<iSaxWord> isax_max(num_channels);

    for (size_t c = 0; c < num_channels; ++c) {
        isax_max[c] = iSaxWord(m_envelopes[0][c].second, isax_word_settings);
        for (size_t i = 1; i < num_envelopes; ++i) {
            isax_max[c].select_max_symbols(iSaxWord(m_envelopes[i][c].second, isax_word_settings));
        }
    }

    auto finalized = std::make_unique<iSaxFinalizedLeaf>(m_file_positions);

    return std::make_pair(std::move(finalized), std::move(isax_max));
}

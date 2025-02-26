#include <utility>

#include "Search/iSax/iSaxSplittableNode.hpp"
#include "Search/iSax/iSaxNode.hpp"
#include "Search/iSax/iSaxFinalizedNode.hpp"
#include "Summarization/Envelope.hpp"
#include "Summarization/iSaxWord.hpp"
#include "Util/typedefs.hpp"

// iSaxSplittableInternal
template <>
uptr<EnvelopeFinalizationResult> iSaxSplittableInternal<EnvelopeTag>::finalize(
    const iSaxWordSettings &isax_word_settings) {
    assert(m_left && m_right);

    auto fin_result_left = static_cast<EnvelopeFinalizationResult *>(m_left->finalize(isax_word_settings));
    auto finalized_left = std::move(fin_result_left->finalized_node);
    auto isax_max_left = std::move(fin_result_left->isax_max);

    auto fin_result_right = static_cast<EnvelopeFinalizationResult *>(m_right->finalize(isax_word_settings));
    auto finalized_right = std::move(fin_result_right->finalized_node);
    auto isax_max_right = std::move(fin_result_right->isax_max);

    bool left_empty = isax_max_left.empty(), right_empty = isax_max_right.empty();

    SaxSymbolT max_symbol_left = left_empty ? 0 : isax_max_left[channel_ind][seg_ind];
    SaxSymbolT max_symbol_right = right_empty ? 0 : isax_max_right[channel_ind][seg_ind];

    uptr<iSaxFinalizedInternal<EnvelopeTag>> finalized = std::make_unique<iSaxEnvelopeFinalizedInternal>(
        m_split_ind, max_symbol_left, max_symbol_right, std::move(finalized_left), std::move(finalized_right));

    // delete children
    m_left.reset();
    m_right.reset();

    if (!left_empty) {
        if (!right_empty) {
            for (size_t c = 0; c < isax_max_left.size(); ++c) {
                isax_max_left[c].select_max_symbols(isax_max_right[c]);
            }
        }
        return std::make_pair(std::move(finalized), std::move(isax_max_left));
    } else {
        return std::make_pair(std::move(finalized), std::move(isax_max_right));
    }
};

// iSaxSplittableLeaf
template <>
uptr<EnvelopeFinalizationResult> iSaxSplittableLeaf<EnvelopeTag>::finalize(const iSaxWordSettings &isax_word_settings) {
    if (m_summaries.size() > 0) {
        assert(m_summaries[0].size() > 0);

        size_t num_envelopes = m_summaries.size(), num_channels = m_summaries[0].size(),
               num_segments = m_summaries[0][0].upper.size();

        vec<iSaxWord> isax_max(num_channels);

        for (size_t c = 0; c < num_channels; ++c) {
            isax_max[c] = iSaxWord(m_summaries[0][c].upper, isax_word_settings);
            for (size_t i = 1; i < num_envelopes; ++i) {
                isax_max[c].select_max_symbols(iSaxWord(m_summaries[i][c].upper, isax_word_settings));
            }
        }
        uptr<iSaxFinalizedLeaf<EnvelopeTag>> finalized =
            std::make_unique<iSaxEnvelopeFinalizedLeaf>(m_subsequence_positions);
        return std::make_pair(std::move(finalized), std::move(isax_max));
    } else {
        uptr<iSaxFinalizedLeaf<EnvelopeTag>> finalized =
            std::make_unique<iSaxEnvelopeFinalizedLeaf>(m_subsequence_positions);
        return std::make_pair(std::move(finalized), vec<iSaxWord>{});
    }
}

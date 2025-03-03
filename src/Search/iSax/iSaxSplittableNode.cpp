#include <utility>

#include "Search/iSax/iSaxSplittableNode.hpp"
#include "Search/iSax/iSaxNode.hpp"
#include "Search/iSax/iSaxFinalizedNode.hpp"
#include "Summarization/Envelope.hpp"
#include "Summarization/iSaxWord.hpp"
#include "Summarization/Paa.hpp"
#include "Util/typedefs.hpp"

uptr<iSaxFinalizedNode<PaaTag>> get_paa_node_finalization_result(uptr<iSaxSplittableNode<Paa>> &node,
                                                                 const iSaxWordSettings &isax_word_settings) {
    auto finalization_result_ptr = node->finalize(isax_word_settings);
    auto finalization_result = static_cast<PaaFinalizationResult *>(finalization_result_ptr.get());
    return std::move(finalization_result->finalized_node);
}

std::pair<uptr<iSaxFinalizedNode<EnvelopeTag>>, vec<iSaxWord>> get_envelope_node_finalization_result(
    uptr<iSaxSplittableNode<Envelope>> &node, const iSaxWordSettings &isax_word_settings) {
    auto finalization_result_ptr = node->finalize(isax_word_settings);
    auto finalization_result = static_cast<EnvelopeFinalizationResult *>(finalization_result_ptr.get());
    auto finalized_node = std::move(finalization_result->finalized_node);
    auto isax_max = std::move(finalization_result->isax_max);
    return {std::move(finalized_node), std::move(isax_max)};
}

// iSaxSplittableInternal<Paa>

template <>
uptr<FinalizationResult> iSaxSplittableInternal<Paa>::finalize(const iSaxWordSettings &isax_word_settings) {
    assert(m_left && m_right);

    auto finalized_left = get_paa_node_finalization_result(m_left, isax_word_settings);
    auto finalized_right = get_paa_node_finalization_result(m_right, isax_word_settings);

    auto args = std::make_unique<iSaxInternalNodeArgs<PaaTag>>(m_split_ind, std::move(finalized_left),
                                                               std::move(finalized_right));
    auto finalized = std::make_unique<iSaxFinalizedInternal<PaaTag>>(std::move(args));

    return std::make_unique<PaaFinalizationResult>(std::move(finalized));
}

template <>
uptr<FinalizationResult> iSaxSplittableLeaf<Paa>::finalize(const iSaxWordSettings &isax_word_settings) {
    uptr<iSaxFinalizedLeaf<PaaTag>> finalized = std::make_unique<iSaxFinalizedLeaf<PaaTag>>(m_subsequence_infos);
    return std::make_unique<PaaFinalizationResult>(std::move(finalized));
}

// iSaxSplittableInternal<Envelope>

template <>
uptr<FinalizationResult> iSaxSplittableInternal<Envelope>::finalize(const iSaxWordSettings &isax_word_settings) {
    assert(m_left && m_right);

    auto [finalized_left, isax_max_left] = get_envelope_node_finalization_result(m_left, isax_word_settings);
    auto [finalized_right, isax_max_right] = get_envelope_node_finalization_result(m_right, isax_word_settings);

    bool left_empty = isax_max_left.empty(), right_empty = isax_max_right.empty();

    auto [seg_ind, channel_ind] = m_split_ind;
    SaxSymbolT max_symbol_left = left_empty ? 0 : isax_max_left[channel_ind][seg_ind];
    SaxSymbolT max_symbol_right = right_empty ? 0 : isax_max_right[channel_ind][seg_ind];

    auto args = std::make_unique<iSaxEnvelopeInternalNodeArgs>(m_split_ind, max_symbol_left, max_symbol_right,
                                                               std::move(finalized_left), std::move(finalized_right));
    auto finalized = std::make_unique<iSaxFinalizedInternal<EnvelopeTag>>(std::move(args));

    // delete children
    m_left.reset();
    m_right.reset();

    if (!left_empty) {
        if (!right_empty) {
            for (size_t c = 0; c < isax_max_left.size(); ++c) {
                isax_max_left[c].select_max_symbols(isax_max_right[c]);
            }
        }
        return std::make_unique<EnvelopeFinalizationResult>(std::move(finalized), std::move(isax_max_left));
    } else {
        return std::make_unique<EnvelopeFinalizationResult>(std::move(finalized), std::move(isax_max_right));
    }
};

// iSaxSplittableLeaf<Envelope>

template <>
uptr<FinalizationResult> iSaxSplittableLeaf<Envelope>::finalize(const iSaxWordSettings &isax_word_settings) {
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
            std::make_unique<iSaxFinalizedLeaf<EnvelopeTag>>(m_subsequence_infos);
        return std::make_unique<EnvelopeFinalizationResult>(std::move(finalized), std::move(isax_max));
    } else {
        uptr<iSaxFinalizedLeaf<EnvelopeTag>> finalized =
            std::make_unique<iSaxFinalizedLeaf<EnvelopeTag>>(m_subsequence_infos);
        return std::make_unique<EnvelopeFinalizationResult>(std::move(finalized), vec<iSaxWord>{});
    }
}

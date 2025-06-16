#include "Util/Stats/IndexAnalyzer.hpp"

#include "Index/ChainIndex/FinalizedChainIndex.hpp"
#include "Index/Entry/SaxEnvelope.hpp"
#include "Index/EnvelopeIndex/Flat/FinalizedFlatEnvelopeIndex.hpp"
#include "Index/Index.hpp"
#include "Index/LengthGroupingIndex/LengthGroupingIndex.hpp"
#include "Index/iSaxIndex/FinalizedISaxIndex.hpp"

// iSAX

template <typename IndexType, typename FTag, typename EnvT>
    requires ValidIndexType<IndexType, FTag>
void IndexAnalyzer<IndexType, FTag, EnvT>::analyze_isax_node(const FinalizedISaxIndex<FTag> *index,
                                                             const FinalizedISaxNode<FTag> *node,
                                                             const vec<typename SaxTraits<FTag>::iSaxType> &isax_words,
                                                             IndexStats &stats, size_t height) {
    if (node->is_leaf()) {
        size_t num_entries = node->get_subsequence_infos().size();
        stats.update_leaf_stats(num_entries, height);
        for (MtsNumChannelsT c = 0; c < isax_words.size(); ++c) {
            auto channel_num_bits = isax_words[c].get_num_bits();
            for (SaxSegIndT s = 0; s < isax_words[c].size(); ++s) {
                auto [lower, upper] = index->get_interval_limits(channel_num_bits[s], isax_words[c].symbol_no_shift(s));
                stats.update_seg_stats(lower, upper, c, s, num_entries);
            }
        }
    } else {
        auto [s, c] = node->get_split_ind();
        auto [left_isax_words, right_isax_words] = index->get_children_isax_words(node, isax_words, c, s);
        auto [left, right] = node->get_children();
        analyze_isax_node(index, left, left_isax_words, stats, height + 1);
        analyze_isax_node(index, right, right_isax_words, stats, height + 1);
    }
}

template <typename IndexType, typename FTag, typename EnvT>
    requires ValidIndexType<IndexType, FTag>
void IndexAnalyzer<IndexType, FTag, EnvT>::analyze_isax(uint length_group_id, bool separate_segment_stats) {
    const FinalizedISaxIndex<FTag> *index = dynamic_cast<FinalizedISaxIndex<FTag> *>(m_index.get());
    if (!index) throw std::runtime_error("Could not cast index to FinalizedISaxIndex");

    const auto &first_layer_symbols = index->get_first_layer_symbols();
    if (first_layer_symbols.empty()) {
        throw std::runtime_error("First layer symbols are empty");
    }
    MtsNumChannelsT num_channels = static_cast<MtsNumChannelsT>(first_layer_symbols[0].size());
    if (num_channels == 0) {
        throw std::runtime_error("Number of channels is 0");
    }
    SaxSegIndT num_segments = static_cast<SaxSegIndT>(first_layer_symbols[0][0].size());
    if (num_segments == 0) {
        throw std::runtime_error("Number of segments is 0");
    }
    IndexStats stats(num_channels, num_segments);

    for (size_t i = 0; i < first_layer_symbols.size(); ++i) {
        vec<iSaxType> isax_words(num_channels);
        for (MtsNumChannelsT c = 0; c < num_channels; ++c) {
            isax_words[c] = iSaxType(first_layer_symbols[i][c], index->get_first_layer_num_bits());
        }
        analyze_isax_node(index, index->get_first_layer_node(i), isax_words, stats, 1);
    }
    stats.calculate();
    IndexStatsLogger::write_entry(stats, length_group_id, m_sub_index_id);
}

template <>
void IndexAnalyzer<FinalizedISaxIndex<PaaTag>, PaaTag>::analyze(uint length_group_id, bool separate_segment_stats) {
    analyze_isax(length_group_id, separate_segment_stats);
}

// iSAX + envelope

template <>
void IndexAnalyzer<FinalizedISaxIndex<EnvelopeTag>, EnvelopeTag>::analyze(uint length_group_id,
                                                                          bool separate_segment_stats) {
    analyze_isax(length_group_id, separate_segment_stats);
}

// Envelope / SAX envelope

template <typename IndexType, typename FTag, typename EnvT>
    requires ValidIndexType<IndexType, FTag>
void IndexAnalyzer<IndexType, FTag, EnvT>::analyze_flat_envelope(uint length_group_id, bool separate_segment_stats) {
    const FinalizedFlatEnvelopeIndex<EnvT> *index = dynamic_cast<FinalizedFlatEnvelopeIndex<EnvT> *>(m_index.get());
    if (!index) throw std::runtime_error("Could not cast index to FinalizedFlatEnvelopeIndex");

    uint num_entries = index->size();
    if (num_entries == 0) {
        throw std::runtime_error("The list of entries is empty");
    }
    MtsNumChannelsT num_channels = static_cast<MtsNumChannelsT>(index->get_entry(0).m_mts_summary.size());
    if (num_channels == 0) {
        throw std::runtime_error("Number of channels is 0");
    }
    SaxSegIndT num_segments = static_cast<SaxSegIndT>(index->get_entry(0).m_mts_summary[0].size());
    if (num_segments == 0) {
        std::cout << "Warning: Number of segments is 0\n";
    }
    IndexStats stats(num_channels, num_segments, separate_segment_stats);

    for (uint i = 0; i < num_entries; ++i) {
        const auto &entry = index->get_entry(i);
        stats.update_leaf_stats(1, 1);
        for (MtsNumChannelsT c = 0; c < num_channels; ++c) {
            auto &summary = entry.m_mts_summary[c];
            for (SaxSegIndT s = 0; s < summary.size(); ++s) {
                Real lower = summary.m_lower[s], upper = summary.m_upper[s];
                stats.update_seg_stats(lower, upper, c, s);
            }
        }
    }
    stats.calculate();
    IndexStatsLogger::write_entry(stats, length_group_id, m_sub_index_id, separate_segment_stats);
}

template <>
void IndexAnalyzer<FinalizedFlatEnvelopeIndex<Envelope>, EnvelopeTag, Envelope>::analyze(uint length_group_id,
                                                                                         bool separate_segment_stats) {
    analyze_flat_envelope(length_group_id, separate_segment_stats);
}

template <>
void IndexAnalyzer<FinalizedFlatEnvelopeIndex<SaxEnvelope>, EnvelopeTag, SaxEnvelope>::analyze(
    uint length_group_id, bool separate_segment_stats) {
    analyze_flat_envelope(length_group_id, separate_segment_stats);
}

// Two-stage iSAX w envelope / SAX envelope

template <>
uptr<FinalizedChainIndex<EnvelopeTag>>
IndexAnalyzer<FinalizedChainIndex<EnvelopeTag>, EnvelopeTag, Envelope>::create_index() {
    vec<uptr<IFinalizedIndex<EnvelopeTag>>> approx_indexes(1);
    approx_indexes[0] = std::make_unique<FinalizedISaxIndex<EnvelopeTag>>();
    uptr<IFinalizedIndex<EnvelopeTag>> exact_index = std::make_unique<FinalizedFlatEnvelopeIndex<Envelope>>();
    return std::make_unique<FinalizedChainIndex<EnvelopeTag>>(std::move(approx_indexes), std::move(exact_index));
}

template <>
uptr<FinalizedChainIndex<EnvelopeTag>>
IndexAnalyzer<FinalizedChainIndex<EnvelopeTag>, EnvelopeTag, SaxEnvelope>::create_index() {
    vec<uptr<IFinalizedIndex<EnvelopeTag>>> approx_indexes(1);
    approx_indexes[0] = std::make_unique<FinalizedISaxIndex<EnvelopeTag>>();
    uptr<IFinalizedIndex<EnvelopeTag>> exact_index = std::make_unique<FinalizedFlatEnvelopeIndex<SaxEnvelope>>();
    return std::make_unique<FinalizedChainIndex<EnvelopeTag>>(std::move(approx_indexes), std::move(exact_index));
}

template <typename IndexType, typename FTag, typename EnvT>
    requires ValidIndexType<IndexType, FTag>
void IndexAnalyzer<IndexType, FTag, EnvT>::analyze_two_stage_isax_envelope(uint length_group_id,
                                                                           bool separate_segment_stats) {
    auto approx_index = uptr<FinalizedISaxIndex<EnvelopeTag>>(
        static_cast<FinalizedISaxIndex<EnvelopeTag> *>(m_index->release_approx_index(0)));
    IndexAnalyzer<FinalizedISaxIndex<EnvelopeTag>, EnvelopeTag>(std::move(approx_index), 0u)
        .analyze(length_group_id, separate_segment_stats);

    auto exact_index = uptr<FinalizedFlatEnvelopeIndex<EnvT>>(
        static_cast<FinalizedFlatEnvelopeIndex<EnvT> *>(m_index->release_exact_index()));
    IndexAnalyzer<FinalizedFlatEnvelopeIndex<EnvT>, EnvelopeTag, EnvT>(std::move(exact_index), 1u)
        .analyze(length_group_id, separate_segment_stats);
}

template <>
void IndexAnalyzer<FinalizedChainIndex<EnvelopeTag>, EnvelopeTag, Envelope>::analyze(uint length_group_id,
                                                                                     bool separate_segment_stats) {
    analyze_two_stage_isax_envelope(length_group_id, separate_segment_stats);
}

template <>
void IndexAnalyzer<FinalizedChainIndex<EnvelopeTag>, EnvelopeTag, SaxEnvelope>::analyze(uint length_group_id,
                                                                                        bool separate_segment_stats) {
    analyze_two_stage_isax_envelope(length_group_id, separate_segment_stats);
}

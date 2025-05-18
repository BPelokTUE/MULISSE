#include "Index/iSaxIndex/SplitStrategy/EntropyMaximizingSplitStrategy.hpp"

#include "Index/Sax/iSaxWord.hpp"
#include "Index/Traits/EntryDataSpec.hpp"
#include "Index/iSaxIndex/SplittableISaxNode.hpp"
#include "Util/RunSettings/RunSettings.hpp"
#include "Util/Types/SaxSplitIndex.hpp"

template <typename T>
EntropyMaximizingSplitStrategy<T>::EntropyMaximizingSplitStrategy(bool choose_min_num_bits_when_tied)
    : m_choose_min_num_bits_when_tied(choose_min_num_bits_when_tied) {}

template <typename T>
SaxSplitIndex EntropyMaximizingSplitStrategy<T>::get_split_ind(const iSaxSplittableLeaf<T> *leaf,
                                                               const vec<iSaxWord> &isax_words) {
    auto &RS = RunSettings::get_instance();

    const vec<Real> &breakpoints = RS.get_breakpoints();

    SaxSplitIndex split_ind{0, 0};
    Real max_score = -INF;
    SaxNumBitsT min_num_bits = RS.get_breakpoint_props().m_breakpoint_num_bits;

    const vec<vec<T>> &summaries = leaf->get_summaries();
    for (MtsNumChannelsT c = 0; c < RS.get_dataset_props().m_num_channels; ++c) {
        vec<SaxNumBitsT> num_bits = isax_words[c].get_num_bits();
        SaxSegIndT num_segments = static_cast<SaxSegIndT>(summaries[0][c].get_isax_input().size());

        for (SaxSegIndT s = 0; s < num_segments; ++s) {
            Real sum = 0, sum_sq = 0, score = 0;

            std::optional<Real> mid_breakpoint = isax_words[c].get_mid_breakpoint(s, breakpoints);
            if (!mid_breakpoint) continue;

            // All values either fall within the lower or the upper part of the segment of the current interval
            uint count_lower = 0;
            for (uint i = 0; i < summaries.size(); ++i)
                if (summaries[i][c].get_isax_input()[s] < mid_breakpoint) ++count_lower;

            Real prob_lower = R(count_lower) / R(summaries.size()), prob_upper = R(1.0) - prob_lower;
            score = -prob_lower * R(log(prob_lower)) - prob_upper * R(log(prob_upper));
            score *= calculate_mu_and_sigma(sum, sum_sq, U(summaries.size())).second;

            if (score > max_score ||
                (m_choose_min_num_bits_when_tied && score == max_score && num_bits[s] < min_num_bits)) {
                max_score = score;
                min_num_bits = num_bits[s];
                split_ind = {s, c};
            }
        }
    }
    return split_ind;
}

DECLARE_ENTRY_DATA_SPECS(EntropyMaximizingSplitStrategy)

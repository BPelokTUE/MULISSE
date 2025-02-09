#include <algorithm>

#include "Search/iSax/iSaxSplitStrategy.hpp"
#include "Util/typedefs.hpp"
#include "Util/utilities.hpp"
#include "Util/RunSettings.hpp"

// Round Robin Strategy

DoubleRoundRobinStrategy::DoubleRoundRobinStrategy(SaxSegIndT num_seg_per_channel, MtsNumChannelsT num_channels)
    : m_num_seg_per_channel(num_seg_per_channel), m_num_channels(num_channels) {}

SaxSplitIndT DoubleRoundRobinStrategy::get_split_ind(const iSaxSplittableLeaf *leaf, const vec<iSaxWord> &isax_mins) {
    SaxSplitIndT inds = {m_current_split, m_current_channel};

    m_current_split = (m_current_split + 1) % m_num_seg_per_channel;
    m_current_channel = (m_current_channel + 1) % m_num_channels;

    return inds;
}

// Entropy Maximizing Strategy

EntropyMaximizingStrategy::EntropyMaximizingStrategy(bool choose_min_num_bits_when_tied)
    : m_choose_min_num_bits_when_tied(choose_min_num_bits_when_tied) {}

SaxSplitIndT EntropyMaximizingStrategy::get_split_ind(const iSaxSplittableLeaf *leaf, const vec<iSaxWord> &isax_mins) {
    auto &RS = RunSettings::get_instance();

    const vec<float> &breakpoints = RS.get_breakpoints();
    uint br_ind;

    SaxSplitIndT split_ind{0, 0};
    float max_score = -INF;
    SaxNumBitsT min_num_bits = RS.get_isax_props().m_breakpoint_num_bits;

    const vec<vec<Envelope>> &envelopes = leaf->get_envelopes();
    for (MtsNumChannelsT c = 0; c < RS.get_dataset_props().num_channels; ++c) {
        vec<SaxNumBitsT> num_bits = isax_mins[c].get_num_bits();
        for (SaxSegIndT s = 0; s < RS.get_isax_props().num_segments; ++s) {
            float lower_sum = 0, lower_sum_sq = 0, score = 0;
            uint count = 0;

            uint alphabet_ratio = (breakpoints.size() + 1) / (1 << (num_bits[s] + 1));
            // If this segment already has the maximum allowed cardinality ==> skip
            if (alphabet_ratio == 0) continue;

            br_ind = alphabet_ratio - 1;

            vec<float> lower_vals(envelopes.size());
            for (uint i = 0; i < envelopes.size(); ++i) lower_vals[i] = envelopes[i][c].lower[s];
            std::sort(lower_vals.begin(), lower_vals.end());

            for (float lower : lower_vals) {
                if (br_ind >= breakpoints.size() || lower < breakpoints[br_ind]) {
                    ++count;
                    lower_sum += lower;
                    lower_sum_sq += lower * lower;
                } else {
                    float prob = (float)count / envelopes.size();
                    if (prob > 0) score -= prob * log2(prob);

                    count = 0;
                    br_ind += alphabet_ratio;
                }
            }
            float prob = (float)count / envelopes.size();
            if (prob > 0) score -= prob * log2(prob);

            score /= calculate_mu_and_sigma(lower_sum, lower_sum_sq, envelopes.size()).second;
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

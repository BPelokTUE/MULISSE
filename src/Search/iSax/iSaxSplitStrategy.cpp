#include "Search/iSax/iSaxSplitStrategy.hpp"
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

SaxSplitIndT EntropyMaximizingStrategy::get_split_ind(const iSaxSplittableLeaf *leaf, const vec<iSaxWord> &isax_mins) {
    SaxSplitIndT split_ind{0, 0};
    float max_score = -INF;

    auto &RS = RunSettings::get_instance();
    const vec<float> &breakpoints = RS.get_breakpoints();
    uint br_ind;

    const vec<vec<Envelope>> &envelopes = leaf->get_envelopes();
    for (MtsNumChannelsT c = 0; c < RS.get_dataset_props().num_channels; ++c) {
        vec<SaxNumBitsT> num_bits = isax_mins[c].get_num_bits();
        for (SaxSegIndT s = 0; s < RS.get_isax_props().num_segments; ++s) {
            float lower_sum = 0, lower_sum_sq = 0, score = 0;
            uint count = 0;

            uint alphabet_ratio = (breakpoints.size() + 1) / (1 << (num_bits[s] + 1));
            assert(alphabet_ratio > 0);

            for (uint i = 0; i < envelopes.size(); ++i) {
                float lower = envelopes[i][c].lower[s];

                if (lower < breakpoints[br_ind]) {
                    ++count;
                    lower_sum += lower;
                    lower_sum_sq += lower * lower;
                } else {
                    float prob = (float)count / envelopes.size();
                    score -= prob * log2(prob);
                    count = 0;
                }
            }
            score /= calculate_mu_and_sigma(lower_sum, lower_sum_sq, envelopes.size()).second;
            if (score > max_score) {
                max_score = score;
                split_ind = {s, c};
            }
        }
    }
    return split_ind;
}

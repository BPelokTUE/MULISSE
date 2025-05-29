#include "Index/Segmentation/ChannelSegmentationStrategy/ScoreBased/EnvelopeWidthScores.hpp"

#include "Index/Entry/Envelope.hpp"
#include "Util/HelperFuncs/Conversion.hpp"

EnvelopeWidthScores::EnvelopeWidthScores(MtsNumChannelsT num_channels, Real min_width_update)
    : m_min_width_update(min_width_update) {}

bool EnvelopeWidthScores::update(const vec<Envelope> &mts_envelope) {
    bool sufficient_update = false;
    for (MtsNumChannelsT c = 0; c < mts_envelope.size(); ++c) {
        auto &envelope = mts_envelope[c];
        for (SaxSegIndT s = 0; s < envelope.size(); ++s) {
            Real lower = envelope.m_lower[s], upper = envelope.m_upper[s];
            Real range = upper - lower;

            if (range < m_range_min) m_range_min = range;
            if (range > m_range_max) m_range_max = range;
            Real prev_range_mean = m_sample_count > 1 ? m_range_sums[c] / R(m_sample_count - 1) : 0.0;
            m_range_sums[c] += range;
            Real range_update = m_range_sums[c] / R(m_sample_count) - prev_range_mean;
            sufficient_update |= std::abs(range_update) / (m_range_max - m_range_min) >= m_min_width_update;
        }
    }
    return sufficient_update;
}

vec<Real> EnvelopeWidthScores::get_scores() {
    vec<Real> scores;
    scores.reserve(m_range_sums.size());
    for (Real range_sum : m_range_sums) {
        scores.push_back((range_sum / R(m_sample_count) - m_range_min) / (m_range_max - m_range_min));
    }
    return scores;
}

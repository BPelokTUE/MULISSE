#include "Index/Segmentation/ChannelSegmentationStrategy/ScoreBased/EnvelopeWidthScores.hpp"

#include "Index/Entry/Envelope.hpp"
#include "Util/HelperFuncs/Conversion.hpp"

EnvelopeWidthScores::EnvelopeWidthScores(MtsNumChannelsT num_channels, Real min_width_update)
    : m_min_width_update(min_width_update), m_range_sums(num_channels, 0.0) {}

bool EnvelopeWidthScores::update(const vec<Envelope> &mts_envelope) {
    bool sufficient_update = false;
    ++m_sample_count;
    for (MtsNumChannelsT c = 0; c < mts_envelope.size(); ++c) {
        auto &envelope = mts_envelope[c];
        Real prev_range_mean = m_sample_count > 1 ? m_range_sums[c] / R(m_sample_count - 1) : 0.0;
        for (SaxSegIndT s = 0; s < envelope.size(); ++s) m_range_sums[c] += envelope.m_lower[s] - envelope.m_upper[s];

        Real range_mean = m_range_sums[c] / R(m_sample_count);
        m_range_min = std::min(m_range_min, range_mean);
        m_range_max = std::max(m_range_max, range_mean);
        sufficient_update |= std::abs(range_mean - prev_range_mean) / (m_range_max - m_range_min) >= m_min_width_update;
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

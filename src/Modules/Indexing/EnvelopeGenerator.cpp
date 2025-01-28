#include "Modules/Indexing/EnvelopeGenerator.hpp"

iSaxEnvelopeGenerator::iSaxEnvelopeGenerator(const IndexOptions &opts) : m_opts(opts) {
    auto *params = static_cast<iSaxEnvelopeIndexParams *>(opts.index_params.get());

    m_uli_params = {
        params->pos_per_env,
        params->segment_len,
        m_opts.l_min,
        m_opts.l_max,
    };
    m_envelope_func = opts.normalized ? ulisse_envelope_normalized : ulisse_envelope_raw;
}

void iSaxEnvelopeGenerator::set_mts(const vec<vec<float>> *mts) {
    m_mts = mts;
    m_position = 0;
    series_len = mts->at(0).size();
}

EnvelopeEntry iSaxEnvelopeGenerator::generate_entry() {
    vec<Envelope> mts_envelope(m_opts.num_channels);

    unsigned remaining_len = series_len - m_position;
    if (remaining_len < m_uli_params.l_min) return {mts_envelope, m_position};

    for (MtsNumChannelsT c = 0; c < m_opts.num_channels; ++c) {
        auto &channel = m_mts->at(c);
        std::span<const float> channel_subseq(channel.begin() + m_position, channel.end());
        mts_envelope[c] = m_envelope_func(channel_subseq, m_uli_params);

        unsigned remaining_segments = remaining_len / m_uli_params.segment_len;
        if (remaining_segments < mts_envelope[c].lower.size()) {
            mts_envelope[c].lower.resize(remaining_segments);
            mts_envelope[c].upper.resize(remaining_segments);
        }
    }
    m_position += m_uli_params.pos_per_env;

    return {mts_envelope, m_position - m_uli_params.pos_per_env};
}

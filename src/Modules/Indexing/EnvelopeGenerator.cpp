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
}

EnvelopeEntry iSaxEnvelopeGenerator::generate_entry() {
    vec<UlisseEnvelope> mts_envelope(m_opts.num_channels);

    for (MtsNumChannelsT c = 0; c < m_opts.num_channels; ++c) {
        auto &channel = m_mts->at(c);
        std::span<const float> channel_subseq(channel.begin() + m_position, channel.end());
        mts_envelope[c] = m_envelope_func(channel_subseq, m_uli_params);
    }

    return {mts_envelope, m_position};
}

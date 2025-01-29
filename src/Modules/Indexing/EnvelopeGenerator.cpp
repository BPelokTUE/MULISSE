#include "Modules/Indexing/EnvelopeGenerator.hpp"

iSaxEnvelopeGenerator::iSaxEnvelopeGenerator(const IndexOptions &opts) : m_opts(opts) {
    auto *params = static_cast<iSaxEnvelopeIndexParams *>(opts.index_params.get());

    m_uli_params = {
        params->pos_per_env,
        params->segment_len,
        m_opts.l_min,
        m_opts.l_max,
    };
    // m_envelope_func = opts.normalized ? ulisse_envelope_normalized : ulisse_envelope_raw;
    m_envelope_func = ulisse_envelope_raw;
}

vec<EnvelopeEntry> iSaxEnvelopeGenerator::get_entries(const vec<vec<float>> &mts) {
    unsigned series_len = mts[0].size();
    unsigned num_env = (series_len - m_uli_params.l_min + m_uli_params.pos_per_env) / m_uli_params.pos_per_env;
    vec<EnvelopeEntry> entries(num_env);

    auto channel_envs = m_envelope_func(mts[0], m_uli_params);
    for (size_t i = 0; i < channel_envs.size(); ++i) {
        size_t file_pos = i * m_uli_params.pos_per_env;

        unsigned remaining_segments = (series_len - file_pos) / m_uli_params.segment_len;
        if (remaining_segments < channel_envs[i].size()) channel_envs[i].resize(remaining_segments);

        entries[i].mts_envelope.resize(m_opts.num_channels);
        entries[i].mts_envelope[0] = std::move(channel_envs[i]);
        entries[i].file_position = file_pos;
    }

    for (MtsNumChannelsT c = 1; c < m_opts.num_channels; ++c) {
        channel_envs = m_envelope_func(mts[c], m_uli_params);
        for (size_t i = 0; i < channel_envs.size(); ++i) {
            size_t file_pos = i * m_uli_params.pos_per_env;

            unsigned remaining_segments = (series_len - file_pos) / m_uli_params.segment_len;
            if (remaining_segments < channel_envs[i].size()) channel_envs[i].resize(remaining_segments);

            entries[i].mts_envelope[c] = std::move(channel_envs[i]);
        }
    }

    return entries;
}

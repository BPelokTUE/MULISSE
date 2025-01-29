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

vec<EnvelopeEntry> iSaxEnvelopeGenerator::get_entries(const vec<vec<float>> &mts, size_t series_ind) {
    unsigned series_len = mts[0].size();
    unsigned num_env = (series_len - m_uli_params.l_min + m_uli_params.pos_per_env) / m_uli_params.pos_per_env;
    vec<EnvelopeEntry> entries(num_env);

    for (MtsNumChannelsT c = 0; c < m_opts.num_channels; ++c) {
        auto channel_envs = m_envelope_func(mts[c], m_uli_params);
        for (size_t i = 0; i < channel_envs.size(); ++i) {
            size_t series_pos = i * m_uli_params.pos_per_env;

            if (c == 0) {
                entries[i].file_position = series_ind * m_opts.num_channels * series_len + series_pos;
                entries[i].mts_envelope.resize(m_opts.num_channels);
            }
            entries[i].mts_envelope[c] = std::move(channel_envs[i]);
        }
    }

    return entries;
}

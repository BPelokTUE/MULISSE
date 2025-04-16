#include "Summarization/Paa.hpp"
#include "Util/typedefs.hpp"
#include "Util/utilities.hpp"

vec<Real> paa(const vec<Real> &ts, uint segment_len) {
    uint num_segments = U(ts.size() / segment_len);
    vec<Real> paa(num_segments);

    Real sum, segment_len_r = R(segment_len);
    uint ind = 0, i, j;
    for (i = 0; i < num_segments; ++i) {
        sum = 0;
        for (j = 0; j < segment_len; ++j, ++ind) {
            sum += ts[ind];
        }
        paa[i] = sum / segment_len_r;
    }
    return paa;
}

Paa::Paa(const vec<Real> &paa_values) : m_paa_values(paa_values) {}

size_t Paa::size() const { return m_paa_values.size(); }

void Paa::resize(size_t new_size) { m_paa_values.resize(new_size); }

vec<Real> Paa::get_isax_input() const { return m_paa_values; }

vec<vec<std::tuple<Paa, uint, uint>>> PaaEntryGenerator::get_paa_entries_normalized(const vec<Real> &ts,
                                                                                    const iSaxPaaParams &paa_params) {
    vec<vec<std::tuple<Paa, uint, uint>>> entry_tuple_groups(m_num_len_groups);

    Real sum = 0, sum_sq = 0;
    for (uint last_ind = 0; last_ind < ts.size(); ++last_ind) {
        sum += ts[last_ind];
        sum_sq += ts[last_ind] * ts[last_ind];

        uint min_start_ind = U(std::max(0, static_cast<int>(last_ind - m_paa_params.m_l_max + 1)));
        int max_start_ind = static_cast<int>(last_ind - m_paa_params.m_l_min + 1);

        if (min_start_ind > 0) {
            sum -= ts[min_start_ind - 1];
            sum_sq -= ts[min_start_ind - 1] * ts[min_start_ind - 1];
        }
        Real tmp_sum = sum, tmp_sum_sq = sum_sq;

        for (uint start_ind = min_start_ind; static_cast<int>(start_ind) <= max_start_ind; ++start_ind) {
            uint subs_len = last_ind - start_ind + 1;
            uint length_group =
                get_length_group(subs_len, m_paa_params.m_l_min, m_paa_params.m_l_max, m_num_len_groups);
            auto [mu, sigma] = calculate_mu_and_sigma(tmp_sum, tmp_sum_sq, subs_len);

            vec<Real> subsequence(subs_len);
            for (uint i = 0; i < subs_len; ++i) subsequence[i] = (ts[start_ind + i] - mu) / sigma;
            vec<Real> paa_values = paa(subsequence, paa_params.m_segment_len);
            paa_values.resize(ts.size() / paa_params.m_segment_len, 0.0);

            entry_tuple_groups[length_group].push_back(std::make_tuple(Paa(paa_values), U(start_ind), subs_len));

            tmp_sum -= ts[start_ind];
            tmp_sum_sq -= ts[start_ind] * ts[start_ind];
        }
    }
    return entry_tuple_groups;
}

PaaEntryGenerator::PaaEntryGenerator(MtsNumChannelsT num_channels, const iSaxPaaParams &paa_params, uint num_len_groups)
    : m_num_channels(num_channels), m_paa_params(paa_params), m_num_len_groups(num_len_groups) {}

vec<vec<IndexEntry<Paa>>> PaaEntryGenerator::get_entries(const vec<vec<Real>> &mts, uint series_ind) {
    vec<vec<IndexEntry<Paa>>> entry_groups(m_num_len_groups);

    for (MtsNumChannelsT c = 0; c < m_num_channels; ++c) {
        auto entry_tuple_groups = get_paa_entries_normalized(mts[c], m_paa_params);
        for (uint l_ind = 0; l_ind < m_num_len_groups; ++l_ind) {
            auto &entry_tuples = entry_tuple_groups[l_ind];
            auto &entry_group = entry_groups[l_ind];
            for (uint i = 0; i < entry_tuples.size(); ++i) {
                if (c == 0) {
                    entry_group.push_back(IndexEntry<Paa>{});
                    entry_group.back().m_subs_info = {series_ind, std::get<1>(entry_tuples[i]),
                                                      std::get<2>(entry_tuples[i])};
                    entry_group.back().m_mts_summary.resize(m_num_channels);
                }
                entry_group[i].m_mts_summary[c] = std::move(std::get<0>(entry_tuples[i]));
            }
        }
    }

    return entry_groups;
}

uint PaaEntryGenerator::get_num_len_groups() const { return m_num_len_groups; }

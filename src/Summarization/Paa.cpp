#include "Summarization/Paa.hpp"
#include "Util/typedefs.hpp"
#include "Util/utilities.hpp"
#include "Util/RunSettings.hpp"

vec<Real> paa(const vec<Real> &ts, const ISegmentationStrategy *segmentation_strategy) {
    uint num_segments = segmentation_strategy->get_num_segments(U(ts.size()));
    vec<Real> paa_values(num_segments);

    Real sum;
    uint ts_ind = 0;
    for (SaxSegIndT s = 0; s < num_segments; ++s) {
        sum = 0;
        uint segment_len = segmentation_strategy->get_segment_len(s);
        for (uint i = 0; i < segment_len; ++i, ++ts_ind) sum += ts[ts_ind];
        paa_values[s] = sum / R(segment_len);
    }
    return paa_values;
}

Paa::Paa(const vec<Real> &paa_values) : m_paa_values(paa_values) {}

bool Paa::operator==(const Paa &other) const {
    return m_paa_values == other.m_paa_values;
}

size_t Paa::size() const { return m_paa_values.size(); }

void Paa::resize(size_t new_size) { m_paa_values.resize(new_size); }

const vec<Real> &Paa::get_isax_input() const { return m_paa_values; }

vec<vec<std::tuple<Paa, uint, uint>>> PaaEntryGenerator::get_paa_entries_normalized(const vec<Real> &ts,
                                                                                    const PaaParams &paa_params) {
    auto &RS = RunSettings::get_instance();

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
            uint lg_ind = RS.get_length_group(subs_len);
            auto segmentation_strategy = paa_params.m_lg_segmentation_strategy->get_const_segmentation_strategy(lg_ind);
            auto [mu, sigma] = calculate_mu_and_sigma(tmp_sum, tmp_sum_sq, subs_len);

            vec<Real> subsequence(subs_len);
            for (uint i = 0; i < subs_len; ++i) subsequence[i] = (ts[start_ind + i] - mu) / sigma;
            vec<Real> paa_values = paa(subsequence, segmentation_strategy);

            uint lg_l_max = RS.get_lg_l_max(lg_ind);
            paa_values.resize(segmentation_strategy->get_num_segments(lg_l_max), 0.0);

            entry_tuple_groups[lg_ind].push_back(std::make_tuple(Paa(paa_values), U(start_ind), subs_len));

            tmp_sum -= ts[start_ind];
            tmp_sum_sq -= ts[start_ind] * ts[start_ind];
        }
    }
    return entry_tuple_groups;
}

PaaEntryGenerator::PaaEntryGenerator(MtsNumChannelsT num_channels, const PaaParams &paa_params, uint num_len_groups)
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

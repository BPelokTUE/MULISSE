#include "Summarization/Paa.hpp"
#include "Util/typedefs.hpp"
#include "Util/utilities.hpp"

vec<float> paa(const vec<float> &ts, uint segment_len) {
    uint num_segments = ts.size() / segment_len;
    vec<float> paa(num_segments);

    float sum;
    uint ind = 0, i, j;
    for (i = 0; i < num_segments; ++i) {
        sum = 0;
        for (j = 0; j < segment_len; ++j, ++ind) {
            sum += ts[ind];
        }
        paa[i] = sum / segment_len;
    }
    return paa;
}

Paa::Paa(const vec<float> &paa_values) : paa_values(paa_values) {}

size_t Paa::size() const { return paa_values.size(); }

void Paa::resize(size_t new_size) { paa_values.resize(new_size); }

vec<float> Paa::get_isax_input() const { return paa_values; }

vec<std::tuple<Paa, uint, uint>> PaaEntryGenerator::get_paa_entries_normalized(const vec<float> &ts,
                                                                               const iSaxPaaParams &paa_params) {
    vec<std::tuple<Paa, uint, uint>> entries;

    float sum = 0, sum_sq = 0;
    for (int last_ind = 0; last_ind < ts.size(); ++last_ind) {
        sum += ts[last_ind];
        sum_sq += ts[last_ind] * ts[last_ind];

        int min_start_ind = std::max(0, last_ind - (int)m_paa_params.l_max + 1);
        int max_start_ind = last_ind - (int)m_paa_params.l_min + 1;

        if (min_start_ind > 0) {
            sum -= ts[min_start_ind - 1];
            sum_sq -= ts[min_start_ind - 1] * ts[min_start_ind - 1];
        }
        float tmp_sum = sum, tmp_sum_sq = sum_sq;

        for (int start_ind = min_start_ind; start_ind <= max_start_ind; ++start_ind) {
            int subs_len = last_ind - start_ind + 1;
            auto [mu, sigma] = calculate_mu_and_sigma(tmp_sum, tmp_sum_sq, subs_len);

            vec<float> subsequence(subs_len);
            for (int i = 0; i < subs_len; ++i) subsequence[i] = (ts[start_ind + i] - mu) / sigma;
            vec<float> paa_values = paa(subsequence, paa_params.segment_len);

            entries.push_back(
                std::make_tuple(Paa(paa_values), static_cast<uint>(start_ind), static_cast<uint>(subs_len)));

            tmp_sum -= ts[start_ind];
            tmp_sum_sq -= ts[start_ind] * ts[start_ind];
        }
    }
    return entries;
}

PaaEntryGenerator::PaaEntryGenerator(MtsNumChannelsT num_channels, const iSaxPaaParams &paa_params)
    : m_num_channels(num_channels), m_paa_params(paa_params) {}

vec<IndexEntry<Paa>> PaaEntryGenerator::get_entries(const vec<vec<float>> &mts, uint series_ind) {
    uint series_len = mts[0].size();
    vec<IndexEntry<Paa>> entries;

    for (MtsNumChannelsT c = 0; c < m_num_channels; ++c) {
        auto channel_items = get_paa_entries_normalized(mts[c], m_paa_params);
        for (uint i = 0; i < channel_items.size(); ++i) {
            if (c == 0) {
                entries.push_back(IndexEntry<Paa>{});
                entries.back().subsequence_info = {series_ind, std::get<1>(channel_items[i]),
                                                   std::get<2>(channel_items[i])};
                entries.back().mts_summary.resize(m_num_channels);
            }
            entries[i].mts_summary[c] = std::move(std::get<0>(channel_items[i]));
        }
    }

    return entries;
}

#include "Index/EntryGenerator/PaaEntryGenerator.hpp"

#include "Index/Entry/IndexEntry.hpp"
#include "Index/Segmentation/ChannelSegmentationStrategy/ChannelSegmentationStrategy.hpp"
#include "Index/Segmentation/LengthGroupSegmentationStrategy/LengthGroupSegmentationStrategy.hpp"
#include "Index/Segmentation/SegmentationStrategy/SegmentationStrategy.hpp"
#include "Util/HelperFuncs/Conversion.hpp"
#include "Util/HelperFuncs/Math.hpp"
#include "Util/RunSettings/RunSettings.hpp"
#include "Util/Types/Numbers.hpp"

PaaEntryGenerator::PaaEntryGenerator(const PaaParams &paa_params, uint num_len_groups)
    : m_paa_params(paa_params), m_num_len_groups(num_len_groups) {}

vec<vec<IndexEntry<Paa>>> PaaEntryGenerator::get_entries(const vec<vec<Real>> &mts, uint series_ind) {
    vec<vec<IndexEntry<Paa>>> entry_groups(m_num_len_groups);

    for (MtsNumChannelsT c = 0; c < mts.size(); ++c) {
        auto entry_tuple_groups = get_paa_entries_normalized(mts[c], c);
        for (uint l_ind = 0; l_ind < m_num_len_groups; ++l_ind) {
            auto &entry_tuples = entry_tuple_groups[l_ind];
            auto &entry_group = entry_groups[l_ind];
            for (uint i = 0; i < entry_tuples.size(); ++i) {
                if (c == 0) {
                    entry_group.push_back(IndexEntry<Paa>{});
                    entry_group.back().m_subs_info = {series_ind, std::get<1>(entry_tuples[i]),
                                                      std::get<2>(entry_tuples[i])};
                    entry_group.back().m_mts_summary.resize(mts.size());
                }
                entry_group[i].m_mts_summary[c] = std::move(std::get<0>(entry_tuples[i]));
            }
        }
    }

    return entry_groups;
}

vec<vec<std::tuple<Paa, uint, uint>>> PaaEntryGenerator::get_paa_entries_normalized(const vec<Real> &ts,
                                                                                    MtsNumChannelsT ch_ind) {
    auto &RS = RunSettings::get_instance();
    auto [l_min, l_max, lg_segmentation_strategy] = m_paa_params;

    vec<vec<std::tuple<Paa, uint, uint>>> entry_tuple_groups(m_num_len_groups);

    vec<Real> sum_accs(ts.size() + 1, 0.0), sq_sum_accs(ts.size() + 1, 0.0);

    for (uint last_ind = 0; last_ind < ts.size(); ++last_ind) {
        sum_accs[last_ind + 1] = sum_accs[last_ind] + ts[last_ind];
        sq_sum_accs[last_ind + 1] = sq_sum_accs[last_ind] + ts[last_ind] * ts[last_ind];

        uint start_min = U(std::max(0, static_cast<int>(last_ind + 1 - l_max)));
        int start_max = static_cast<int>(last_ind + 1 - l_min);

        for (uint first_ind = start_min; static_cast<int>(first_ind) <= start_max; ++first_ind) {
            uint subs_len = last_ind - first_ind + 1;
            auto [mu, sigma] = calculate_mu_and_sigma(sum_accs[last_ind + 1] - sum_accs[first_ind],
                                                      sq_sum_accs[last_ind + 1] - sq_sum_accs[first_ind], subs_len);

            uint length_group = RS.get_length_group(subs_len);
            uint lg_l_max = RS.get_lg_l_max(length_group);
            auto segmentation_strategy = lg_segmentation_strategy->get_const_ch_segmentation_strategy(length_group)
                                             ->get_const_segmentation_strategy(ch_ind);

            SaxSegIndT num_segments = segmentation_strategy->get_num_segments(subs_len),
                       max_num_segments = segmentation_strategy->get_num_segments(lg_l_max);

            uint segment_len_sum = 0;
            vec<Real> paa_vals(max_num_segments, 0.0);
            for (SaxSegIndT seg_ind = 0; seg_ind < num_segments; ++seg_ind) {
                uint segment_len = segmentation_strategy->get_segment_len(seg_ind);
                Real paa_val =
                    (sum_accs[first_ind + segment_len_sum + segment_len] - sum_accs[first_ind + segment_len_sum]) /
                    R(segment_len);
                segment_len_sum += segment_len;
                paa_val = (paa_val - mu) / sigma;

                paa_vals[seg_ind] = paa_val;
            }
            entry_tuple_groups[length_group].push_back(std::make_tuple(Paa(paa_vals), first_ind, subs_len));
        }
    }
    return entry_tuple_groups;
}

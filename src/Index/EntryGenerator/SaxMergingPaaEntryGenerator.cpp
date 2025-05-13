#include "Index/EntryGenerator/SaxMergingPaaEntryGenerator.hpp"

#include "Index/Sax/SaxHelpers.hpp"
#include "Util/RunSettings/RunSettings.hpp"

SaxMergingPaaEntryGenerator::SaxMergingPaaEntryGenerator(const PaaParams &paa_params, SaxNumBitsT merger_num_bits,
                                                         uint num_len_groups)
    : m_paa_params(paa_params),
      m_num_len_groups(num_len_groups),
      m_sax_symbols_factory(RunSettings::get_instance().get_breakpoint_props(), merger_num_bits) {}

vec<vec<IndexEntry<Paa>>> SaxMergingPaaEntryGenerator::get_entries(const vec<vec<Real>> &mts, uint series_ind) {
    auto &RS = RunSettings::get_instance();
    auto [l_min, l_max, lg_segmentation_strategy] = m_paa_params;
    MtsNumChannelsT num_channels = static_cast<MtsNumChannelsT>(mts.size());
    uint series_len = U(mts[0].size());

    vec<vec<Real>> sum_accs(num_channels, vec<Real>(series_len + 1, 0.0)),
        sq_sum_accs(num_channels, vec<Real>(series_len + 1, 0.0));
    for (MtsNumChannelsT c = 0; c < num_channels; ++c) {
        auto &channel = mts[c];
        for (uint ind = 0; ind < series_len; ++ind) {
            sum_accs[c][ind + 1] = sum_accs[c][ind] + channel[ind];
            sq_sum_accs[c][ind + 1] = sq_sum_accs[c][ind] + channel[ind] * channel[ind];
        }
    }

    // Hash map for each length group, mapping SAX representation to the last entry with that representation in the
    // group
    vec<umap_hash<vec<vec<SaxSymbolT>>, IndexEntry<Paa>, SaxSymbolsHash>> symbols_to_entry(m_num_len_groups);

    vec<vec<IndexEntry<Paa>>> entry_groups(m_num_len_groups);

    uint num_start_pos = series_len - l_min + 1;
    for (uint first_ind = 0; first_ind < num_start_pos; ++first_ind) {
        uint max_ind = std::min(first_ind + l_max - 1, U(series_len - 1));
        for (uint last_ind = first_ind + l_min - 1; last_ind <= max_ind; ++last_ind) {
            uint subs_len = last_ind - first_ind + 1;
            uint length_group = RS.get_length_group(subs_len);
            auto segmentation_strategy = lg_segmentation_strategy->get_const_segmentation_strategy(length_group);
            SaxSegIndT lg_num_segments = segmentation_strategy->get_num_segments(RS.get_lg_l_max(length_group));

            vec<vec<SaxSymbolT>> symbols(num_channels);
            vec<Paa> paas(num_channels, Paa(lg_num_segments));

            // Calculate PAA values and SAX symbols
            for (MtsNumChannelsT c = 0; c < num_channels; ++c) {
                auto [mu, sigma] =
                    calculate_mu_and_sigma(sum_accs[c][last_ind + 1] - sum_accs[c][first_ind],
                                           sq_sum_accs[c][last_ind + 1] - sq_sum_accs[c][first_ind], subs_len);

                SaxSegIndT num_segments = segmentation_strategy->get_num_segments(subs_len);

                uint segment_len_sum = 0;
                for (SaxSegIndT s = 0; s < num_segments; ++s) {
                    uint segment_len = segmentation_strategy->get_segment_len(s);
                    Real paa_val = (sum_accs[c][first_ind + segment_len_sum + segment_len] -
                                    sum_accs[c][first_ind + segment_len_sum]) /
                                   R(segment_len);
                    segment_len_sum += segment_len;
                    paas[c][s] = (paa_val - mu) / sigma;
                }
                symbols[c] = m_sax_symbols_factory.get_symbols(paas[c].m_paa_values);
            }

            // Merge
            auto it = symbols_to_entry[length_group].find(symbols);
            if (it == symbols_to_entry[length_group].end()) {
                symbols_to_entry[length_group][symbols] = {{series_ind, first_ind, subs_len}, std::move(paas)};
            } else {
                auto &last_entry = it->second;
                uint last_entry_rightmost = last_entry.m_subs_info.m_start_pos + last_entry.m_subs_info.m_length - 1;
                if (first_ind > last_entry_rightmost + 1) {
                    entry_groups[length_group].push_back(std::move(last_entry));
                    symbols_to_entry[length_group][symbols] = {{series_ind, first_ind, subs_len}, std::move(paas)};
                } else if (last_entry_rightmost < last_ind) {
                    last_entry.m_subs_info.m_length = last_ind - last_entry.m_subs_info.m_start_pos + 1;
                }
            }
        }
    }

    // Add the remaining entries to the entry groups
    for (uint lg_ind = 0; lg_ind < m_num_len_groups; ++lg_ind) {
        for (auto &[symbols, entry] : symbols_to_entry[lg_ind]) {
            entry_groups[lg_ind].push_back(std::move(entry));
        }
    }

    return entry_groups;
}

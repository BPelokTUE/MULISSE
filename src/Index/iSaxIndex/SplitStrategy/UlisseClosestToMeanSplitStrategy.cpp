#include "Index/iSaxIndex/SplitStrategy/UlisseClosestToMeanSplitStrategy.hpp"

#include "Index/Sax/iSaxWord.hpp"
#include "Index/Traits/EntryDataSpec.hpp"
#include "Index/iSaxIndex/SplittableISaxNode.hpp"
#include "Util/RunSettings/RunSettings.hpp"
#include "Util/Types/SaxSplitIndex.hpp"

template <typename T>
UlisseClosestToMeanStrategy<T>::UlisseClosestToMeanStrategy(Real max_std_dist) : m_max_std_dist(max_std_dist) {}

template <typename T>
SaxSplitIndex UlisseClosestToMeanStrategy<T>::get_split_ind(const iSaxSplittableLeaf<T> *leaf,
                                                            const vec<iSaxWord> &isax_words) {
    auto &RS = RunSettings::get_instance();

    SaxSplitIndex split_ind{0, 0};
    Real split_breakpoint;
    bool split_ind_set = false;

    const vec<Real> &breakpoints = RS.get_breakpoints();
    MtsNumChannelsT num_channels = static_cast<MtsNumChannelsT>(isax_words.size());
    SaxSegIndT num_segments = static_cast<SaxSegIndT>(isax_words[0].size());

    const vec<vec<T>> &summaries = leaf->get_summaries();
    for (MtsNumChannelsT c = 0; c < num_channels; ++c) {
        vec<SaxNumBitsT> num_bits = isax_words[c].get_num_bits();
        for (SaxSegIndT s = 0; s < num_segments; ++s) {
            Real sum = 0, sum_sq = 0;
            uint count = 0;

            std::optional<Real> mid_breakpoint = isax_words[c].get_mid_breakpoint(s, breakpoints);
            if (!mid_breakpoint) continue;

            for (uint i = 0; i < summaries.size(); ++i) {
                Real input = summaries[i][c].get_isax_input()[s];
                sum += input;
                sum_sq += input * input;
                ++count;
            }
            auto [mu, sigma] = calculate_mu_and_sigma(sum, sum_sq, count);

            if ((*mid_breakpoint - mu) / sigma <= m_max_std_dist &&
                (!split_ind_set || (std::abs(*mid_breakpoint - mu) < std::abs(split_breakpoint - mu)))) {
                split_ind = {s, c};
                split_breakpoint = *mid_breakpoint;
            }
        }
    }

    if (!split_ind_set)
        split_ind = {static_cast<SaxSegIndT>(std::rand() % num_segments),
                     static_cast<MtsNumChannelsT>(std::rand() % num_channels)};
    return split_ind;
}

DECLARE_ENTRY_DATA_SPECS(UlisseClosestToMeanStrategy)

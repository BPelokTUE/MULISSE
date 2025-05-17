#ifndef INDEX_ISAXINDEX_SPLITSTRATEGY_CLOSESTTOMEANSPLITSTRATEGY_HPP
#define INDEX_ISAXINDEX_SPLITSTRATEGY_CLOSESTTOMEANSPLITSTRATEGY_HPP

#include "Index/iSaxIndex/SplitStrategy/iSaxSplitStrategy.hpp"

/**
 * @brief Closest to mean split strategy
 *
 * Split strategy that selects the segment with mean closest to it's breakpoint
 */
template <typename T>
    requires DerivedFromEntryData<T>
class ClosestToMeanStrategy : public IiSaxSplitStrategy<T> {
   public:
    /** @brief Constructor */
    ClosestToMeanStrategy(Real max_std_dist = 3.0) : m_max_std_dist(max_std_dist) {}

    SaxSplitIndex get_split_ind(const iSaxSplittableLeaf<T> *leaf, const vec<iSaxWord> &isax_words) override {
        auto &RS = RunSettings::get_instance();

        SaxSplitIndex split_ind{0, 0};
        Real min_diff = INF;

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

                if ((*mid_breakpoint - mu) / sigma <= m_max_std_dist) {
                    Real diff = abs(*mid_breakpoint - mu);
                    if (diff < min_diff) {
                        split_ind = {s, c};
                        min_diff = diff;
                    }
                }
            }
        }
        if (min_diff == INF)
            split_ind = {static_cast<SaxSegIndT>(std::rand() % num_segments),
                         static_cast<MtsNumChannelsT>(std::rand() % num_channels)};
        return split_ind;
    }

   private:
    Real m_max_std_dist;
};

#endif  // INDEX_ISAXINDEX_SPLITSTRATEGY_CLOSESTTOMEANSPLITSTRATEGY_HPP

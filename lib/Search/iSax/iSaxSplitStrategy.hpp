#ifndef ISAX_SPLIT_STRATEGY_HPP
#define ISAX_SPLIT_STRATEGY_HPP

#include <cstdlib>

#include "Search/iSax/iSaxSplittableNode.hpp"
#include "Summarization/IndexEntry.hpp"
#include "Util/typedefs.hpp"
#include "Util/utilities.hpp"
#include "Util/RunSettings.hpp"

/** @brief Enum for IiSaxSplitStrategy implementations */
enum iSaxSplitStrategyType { DOUBLE_ROUND_ROBIN, ENTROPY_MAXIMIZING, ULISSE_CLOSEST_TO_MEAN, CLOSES_TO_MEAN };

DEFINE_ENUM_CONSTS_NO_EXTRA(iSaxSplitStrategyType, ISAX_SPLIT_STRATEGY, true);

/** @brief Interface for iSAX split strategies */
template <typename T>
    requires DerivedFromEntryData<T>
class IiSaxSplitStrategy {
   public:
    virtual ~IiSaxSplitStrategy() {}

    /**
     * @brief Get a channel and segment index to split on
     * @param leaf The leaf to get the split index for
     * @return A channel and segment index to split on (see SaxSplitIndex)
     */
    virtual SaxSplitIndex get_split_ind(const iSaxSplittableLeaf<T> *leaf, const vec<iSaxWord> &isax_mins) = 0;
};

/**
 * @brief Double round-robin split strategy
 *
 * Split strategy that selects the channel and segment indices using round-robin with separate counters
 * */
template <typename T>
    requires DerivedFromEntryData<T>
class DoubleRoundRobinStrategy : public IiSaxSplitStrategy<T> {
   public:
    /**
     * @brief Constructor
     * @param num_seg_per_channel Number of segments per channel
     * @param num_channels Number of channels
     */
    DoubleRoundRobinStrategy(SaxSegIndT num_seg_per_channel, MtsNumChannelsT num_channels)
        : m_num_seg_per_channel(num_seg_per_channel), m_num_channels(num_channels) {}

    SaxSplitIndex get_split_ind(const iSaxSplittableLeaf<T> *leaf, const vec<iSaxWord> &isax_mins) override {
        SaxSplitIndex inds = {m_current_split, m_current_channel};
        m_current_split = static_cast<SaxSegIndT>((m_current_split + 1) % m_num_seg_per_channel);
        m_current_channel = static_cast<SaxSegIndT>((m_current_channel + 1) % m_num_channels);
        return inds;
    }

   private:
    SaxSegIndT m_num_seg_per_channel, m_current_split = 0;
    MtsNumChannelsT m_num_channels, m_current_channel = 0;
};

/**
 * @brief Entropy maximizing split strategy
 *
 * Split strategy that selects the segment that, when split, maximizes the entropy of
 */
template <typename T>
    requires DerivedFromEntryData<T>
class EntropyMaximizingStrategy : public IiSaxSplitStrategy<T> {
   public:
    /**
     * @brief Constructor
     * @param choose_min_num_bits_when_tied If true, when the score is tied, the segment with the smallest number of
     * bits is chosen
     */
    EntropyMaximizingStrategy(bool choose_min_num_bits_when_tied)
        : m_choose_min_num_bits_when_tied(choose_min_num_bits_when_tied) {}

    SaxSplitIndex get_split_ind(const iSaxSplittableLeaf<T> *leaf, const vec<iSaxWord> &isax_words) override {
        auto &RS = RunSettings::get_instance();

        const vec<Real> &breakpoints = RS.get_breakpoints();

        SaxSplitIndex split_ind{0, 0};
        Real max_score = -INF;
        SaxNumBitsT min_num_bits = RS.get_isax_props().m_breakpoint_num_bits;

        const vec<vec<T>> &summaries = leaf->get_summaries();
        for (MtsNumChannelsT c = 0; c < RS.get_dataset_props().m_num_channels; ++c) {
            vec<SaxNumBitsT> num_bits = isax_words[c].get_num_bits();
            for (SaxSegIndT s = 0; s < RS.get_isax_props().m_num_segments; ++s) {
                Real sum = 0, sum_sq = 0, score = 0;

                std::optional<Real> mid_breakpoint = isax_words[c].get_mid_breakpoint(s, breakpoints);
                if (!mid_breakpoint) continue;

                // All values either fall within the lower or the upper part of the segment of the current interval
                uint count_lower = 0;
                for (uint i = 0; i < summaries.size(); ++i)
                    if (summaries[i][c].get_isax_input()[s] < mid_breakpoint) ++count_lower;

                Real prob_lower = R(count_lower) / R(summaries.size()), prob_upper = R(1.0) - prob_lower;
                score = -prob_lower * log(prob_lower) - prob_upper * log(prob_upper);
                score *= calculate_mu_and_sigma(sum, sum_sq, static_cast<uint>(summaries.size())).second;

                if (score > max_score ||
                    (m_choose_min_num_bits_when_tied && score == max_score && num_bits[s] < min_num_bits)) {
                    max_score = score;
                    min_num_bits = num_bits[s];
                    split_ind = {s, c};
                }
            }
        }
        return split_ind;
    }

   private:
    bool m_choose_min_num_bits_when_tied;
};

/**
 * @brief Closest to mean split strategy in line with ULISSE implementation
 *
 * Split strategy that selects the last segment such that it's mean is closer to it's breakpoint than to the breakpoint
 * of the previously selected segment, and the breakpoint is within `max_std_dist` standard deviations of the mean
 */
template <typename T>
    requires DerivedFromEntryData<T>
class UlisseClosestToMeanStrategy : public IiSaxSplitStrategy<T> {
   public:
    /** @brief Constructor */
    UlisseClosestToMeanStrategy(Real max_std_dist = 3.0) : m_max_std_dist(max_std_dist) {}

    SaxSplitIndex get_split_ind(const iSaxSplittableLeaf<T> *leaf, const vec<iSaxWord> &isax_words) override {
        auto &RS = RunSettings::get_instance();

        SaxSplitIndex split_ind{0, 0};
        Real split_breakpoint;
        bool split_ind_set = false;

        const vec<Real> &breakpoints = RS.get_breakpoints();
        MtsNumChannelsT num_channels = RS.get_dataset_props().m_num_channels;
        SaxSegIndT num_segments = RS.get_isax_props().m_num_segments;

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
                    (!split_ind_set || (abs(*mid_breakpoint - mu) < abs(split_breakpoint - mu)))) {
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

   private:
    Real m_max_std_dist;
};

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
        MtsNumChannelsT num_channels = RS.get_dataset_props().m_num_channels;
        SaxSegIndT num_segments = RS.get_isax_props().m_num_segments;

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

#endif  // ISAX_SPLIT_STRATEGY_HPP

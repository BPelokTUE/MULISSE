#ifndef PAA_HPP
#define PAA_HPP

#include <tuple>

#include "Util/typedefs.hpp"
#include "Summarization/IndexEntry.hpp"
#include "Summarization/SegmentationStrategy.hpp"

/**
 * @brief Piecewise Aggregate Approximation (PAA) of a time series
 *
 * This function computes the Piecewise Aggregate Approximation (PAA) of a time series
 *
 * @param ts The time series to approximate
 * @param segmentation_strategy The segmentation strategy to use
 * @return The PAA of the time series
 */
vec<Real> paa(const vec<Real> &ts, const ISegmentationStrategy *segmentation_strategy);

/**
 * @brief Parameters for the iSAX PAA computation
 *
 * This struct contains the parameters needed to compute the PAAs of subsequences to insert into the iSAX index
 *
 * @param segmentation_strategy The segmentation strategy to use
 * @param l_min The minimum length of a subsequence
 * @param l_max The maximum length of a subsequence
 */
struct iSaxPaaParams {
    uint m_l_min;
    uint m_l_max;
    const ISegmentationStrategy *m_segmentation_strategy;
};

struct Paa : EntryData {
    vec<Real> m_paa_values;

    Paa(const vec<Real> &paa_values);

    Paa() = default;

    size_t size() const override;

    void resize(size_t new_size) override;

    vec<Real> get_isax_input() const override;

    template <class Archive>
    void serialize(Archive &ar) {
        ar(m_paa_values);
    }
};

/** @brief PAA generator for iSAX index */
class PaaEntryGenerator : public IEntryGenerator<Paa> {
   public:
    /**
     * @brief Construct a new PaaEntryGenerator object
     * @param num_channels Number of channels of each series
     * @param paa_params Parameters for the PAA computation
     * @param num_len_groups Number of length groups
     */
    PaaEntryGenerator(MtsNumChannelsT num_channels, const iSaxPaaParams &paa_params, uint num_len_groups = 1);

    vec<vec<IndexEntry<Paa>>> get_entries(const vec<vec<Real>> &mts, uint series_ind) override;

   private:
    MtsNumChannelsT m_num_channels;
    uint m_num_len_groups;
    iSaxPaaParams m_paa_params;

    /**
     * @brief Get the PAA entries for all normalized subsequences of a UTS, grouped by length
     * @param ts The time series
     * @param paa_params The parameters for the PAA computation
     * @return The PAA entries, their time series index and their starting position
     */
    vec<vec<std::tuple<Paa, uint, uint>>> get_paa_entries_normalized(const vec<Real> &ts,
                                                                     const iSaxPaaParams &paa_params);
};

#endif  // PAA_HPP

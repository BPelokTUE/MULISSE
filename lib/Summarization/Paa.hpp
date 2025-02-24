#ifndef PAA_HPP
#define PAA_HPP

#include <vector>

#include "Util/typedefs.hpp"
#include "Summarization/IndexEntry.hpp"

/**
 * @brief Piecewise Aggregate Approximation (PAA) of a time series
 *
 * This function computes the Piecewise Aggregate Approximation (PAA) of a time series
 *
 * @param ts The time series to approximate
 * @param segment_len The length of each segment Assumed to be greater than 0
 * @return The PAA of the time series
 */
vec<float> paa(const vec<float> &ts, uint segment_len);

/**
 * @brief Parameters for the iSAX PAA computation
 *
 * This struct contains the parameters needed to compute the PAAs of subsequences to insert into the iSAX index
 *
 * @param segment_len The length of each PAA segment
 * @param l_min The minimum length of a subsequence
 * @param l_max The maximum length of a subsequence
 */
struct iSaxPaaParams {
    uint segment_len;
    uint l_min;
    uint l_max;
};

using PaaEntry = IndexEntry<vec<float>>;

/** @brief PAA generator for iSAX index */
class iSaxPaaGenerator : public IEntryGenerator<vec<float>> {
   public:
    /**
     * @brief Construct a new iSaxPaaGenerator object
     * @param opts Indexing options
     */
    iSaxPaaGenerator(MtsNumChannelsT num_channels, bool normalized, const iSaxPaaParams &paa_params);

    vec<PaaEntry> get_entries(const vec<vec<float>> &mts, uint series_ind) override;

   private:
    MtsNumChannelsT m_num_channels;
    bool m_normalized;
    iSaxPaaParams m_paa_params;
    vec<vec<float>> (*m_paa_func)(const vec<float> &, const iSaxPaaParams &);
};

#endif  // PAA_HPP

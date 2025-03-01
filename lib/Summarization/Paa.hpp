#ifndef PAA_HPP
#define PAA_HPP

#include <tuple>

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

struct Paa : EntryData {
    vec<float> paa_values;

    Paa(const vec<float> &paa_values);

    Paa() = default;

    size_t size() const override;

    void resize(size_t new_size) override;

    vec<float> get_isax_input() const override;
};

/** @brief PAA generator for iSAX index */
class iSaxPaaGenerator : public IEntryGenerator<Paa> {
   public:
    /**
     * @brief Construct a new iSaxPaaGenerator object
     * @param num_channels Number of channels of each series
     * @param uli_params Parameters for the ULISSE envelope computation
     */
    iSaxPaaGenerator(MtsNumChannelsT num_channels, const iSaxPaaParams &paa_params);

    vec<IndexEntry<Paa>> get_entries(const vec<vec<float>> &mts, uint series_ind) override;

   private:
    MtsNumChannelsT m_num_channels;
    bool m_normalized;
    iSaxPaaParams m_paa_params;

    /**
     * @brief Get the PAA entries for all normalized subsequences of a UTS
     * @param ts The time series
     * @param paa_params The parameters for the PAA computation
     * @return The PAA entries and their starting positions
     */
    vec<std::tuple<Paa, uint, uint>> get_paa_entries_normalized(const vec<float> &ts, const iSaxPaaParams &paa_params);
};

#endif  // PAA_HPP

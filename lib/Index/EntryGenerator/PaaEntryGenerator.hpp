#ifndef INDEX_ENTRYGENERATOR_PAAENTRYGENERATOR_HPP
#define INDEX_ENTRYGENERATOR_PAAENTRYGENERATOR_HPP

#include <tuple>

#include "Index/Entry/IndexEntry.hpp"
#include "Index/Entry/Paa.hpp"
#include "Index/EntryGenerator/EntryGenerator.hpp"
#include "Util/Types/Containers.hpp"
#include "Util/Types/Numbers.hpp"

/** @brief PAA generator for iSAX index */
class PaaEntryGenerator : public IEntryGenerator<Paa> {
   public:
    /**
     * @brief Construct a new PaaEntryGenerator object
     * @param num_channels Number of channels of each series
     * @param paa_params Parameters for the PAA computation
     * @param num_len_groups Number of length groups
     */
    PaaEntryGenerator(MtsNumChannelsT num_channels, const PaaParams &paa_params, uint num_len_groups = 1);

    vec<vec<IndexEntry<Paa>>> get_entries(const vec<vec<Real>> &mts, uint series_ind) override;

   private:
    MtsNumChannelsT m_num_channels;
    uint m_num_len_groups;
    PaaParams m_paa_params;

    /**
     * @brief Get the PAA entries for all normalized subsequences of a UTS, grouped by length
     * @param ts The time series
     * @param paa_params The parameters for the PAA computation
     * @return The PAA entries, their time series index and their starting position
     */
    vec<vec<std::tuple<Paa, uint, uint>>> get_paa_entries_normalized(const vec<Real> &ts, const PaaParams &paa_params);
};

#endif  // INDEX_ENTRYGENERATOR_PAAENTRYGENERATOR_HPP

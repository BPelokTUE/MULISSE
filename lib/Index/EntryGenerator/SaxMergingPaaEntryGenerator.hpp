#ifndef INDEX_ENTRYGENERATOR_SAXMERGINGPAAENTRYGENERATOR_HPP
#define INDEX_ENTRYGENERATOR_SAXMERGINGPAAENTRYGENERATOR_HPP

#include "Index/Entry/Paa.hpp"
#include "Index/EntryGenerator/EntryGenerator.hpp"
#include "Index/Sax/SaxSymbolsFactory.hpp"

/** @brief PAA generator for iSAX index that also merges overlapping entries similar to SaxBasedEntryMerger */
class SaxMergingPaaEntryGenerator : public IEntryGenerator<Paa> {
   public:
    /**
     * @brief Construct a new SaxMergingPaaEntryGenerator object
     * @param paa_params Parameters for the PAA computation
     * @param merger_num_bits The number of bits to use for the SAX representation for merging
     * @param num_len_groups Number of length groups
     */
    SaxMergingPaaEntryGenerator(const PaaParams &paa_params, SaxNumBitsT merger_num_bits, uint num_len_groups = 1);

    vec<vec<IndexEntry<Paa>>> get_entries(const vec<vec<Real>> &mts, uint series_ind) override;

   private:
    uint m_num_len_groups;
    PaaParams m_paa_params;
    SaxSymbolsFactory m_sax_symbols_factory;
};

#endif  // INDEX_ENTRYGENERATOR_SAXMERGINGPAAENTRYGENERATOR_HPP

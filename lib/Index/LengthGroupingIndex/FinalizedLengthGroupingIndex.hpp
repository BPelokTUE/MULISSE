#ifndef INDEX_LENGTHGROUPINGINDEX_FINALIZEDLENGTHGROUPINGINDEX_HPP
#define INDEX_LENGTHGROUPINGINDEX_FINALIZEDLENGTHGROUPINGINDEX_HPP

#include "Index/FinalizedIndex.hpp"
#include "Index/Traits/EntryTags.hpp"
#include "Util/RunSettings/LengthProperties.hpp"
#include "Util/Types/Pointers.hpp"

/**
 * @brief Group of finalized indexes each containing entries summarizing data about subsequences in different length
 * ranges.
 * @tparam FTag The traits of the entries in the index
 */
template <typename FTag>
class FinalizedLengthGroupingIndex : public IFinalizedIndex<FTag> {
   public:
    /**
     * @brief Construct a new FinalizedLengthGroupingIndex object
     * @param indexes The indexes to use for each length group
     * @param length_props The length properties to use for the length groups
     */
    FinalizedLengthGroupingIndex(vec<uptr<IFinalizedIndex<FTag>>> indexes, LengthProperties length_props);

    void save(const str &out_file, ArchiveType ar_type) override;

    void load(const str &in_file, ArchiveType ar_type) override;

    size_t get_size_on_disk(const str &index_file, const ArchiveType ar_type) const override;

    /**
     * @brief Release the index of a specific length group
     * @param length_group The index of the length group
     * @return Pointer to the released index
     */
    IFinalizedIndex<FTag> *release_index(uint length_group);

   private:
    vec<uptr<IFinalizedIndex<FTag>>> m_indexes;
    LengthProperties m_length_props;

    /**
     * @brief Get the file name for a specific length group index.
     * @param length_group The index of the length group
     * @return The file name for the index of the specified length group
     */
    str get_index_file_name(uint length_group) const;
};

#endif  // INDEX_LENGTHGROUPINGINDEX_FINALIZEDLENGTHGROUPINGINDEX_HPP

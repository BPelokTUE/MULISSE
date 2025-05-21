#ifndef INDEX_LENGTHGROUPINGINDEX_FINALIZEDLENGTHGROUPINGINDEX_HPP
#define INDEX_LENGTHGROUPINGINDEX_FINALIZEDLENGTHGROUPINGINDEX_HPP

#include "Index/FinalizedIndex.hpp"
#include "Index/Traits/EntryTags.hpp"
#include "Util/Types/Pointers.hpp"

/**
 * @brief Group of finalized indexes each containing entries summarizing data about subsequences in different length
 * ranges.
 * @tparam FTag The traits of the entries in the index
 */
template <typename FTag>
    requires ValidEntryTraitsTag<FTag>
class FinalizedLengthGroupingIndex : public IFinalizedIndex<FTag> {
   public:
    /**
     * @brief Construct a new FinalizedLengthGroupingIndex object
     * @param indexes The indexes to use for each length group
     * @param l_min Minimum query length
     * @param l_max Maximum query length
     */
    FinalizedLengthGroupingIndex(vec<uptr<IFinalizedIndex<FTag>>> indexes, uint l_min, uint l_max)
        : m_indexes(std::move(indexes)), m_l_min(l_min), m_l_max(l_max) {
        assert(l_min > 0);
        assert(l_max > 0);
        assert(l_min <= l_max);
    }

    void save(const str &out_file, ArchiveType ar_type) override {
        str base = get_file_base_and_extension(out_file).first;

        if (!fs::exists(base)) fs::create_directories(base);

        for (uint l_ind = 0; l_ind < m_indexes.size(); ++l_ind)
            m_indexes[l_ind]->save(fs::path(base) / get_index_file_name(l_ind), ar_type);
    }

    void load(const str &in_file, ArchiveType ar_type) override {
        str base = get_file_base_and_extension(in_file).first;

        for (uint l_ind = 0; l_ind < m_indexes.size(); ++l_ind)
            m_indexes[l_ind]->load(fs::path(base) / get_index_file_name(l_ind), ar_type);
    }

    size_t get_size_on_disk(const str &index_file, const ArchiveType ar_type) const override {
        str base = get_file_base_and_extension(index_file).first;

        size_t size = 0;
        for (uint l_ind = 0; l_ind < m_indexes.size(); ++l_ind)
            size += m_indexes[l_ind]->get_size_on_disk(fs::path(base) / get_index_file_name(l_ind), ar_type);
        return size;
    }

    IFinalizedIndex<FTag> *release_index(uint length_group) {
        assert(length_group < m_indexes.size());
        return m_indexes[length_group].release();
    }

   private:
    vec<uptr<IFinalizedIndex<FTag>>> m_indexes;
    uint m_l_min, m_l_max;

    str get_index_file_name(uint length_group) const { return "LG_" + std::to_string(length_group); }
};

#endif  // INDEX_LENGTHGROUPINGINDEX_FINALIZEDLENGTHGROUPINGINDEX_HPP

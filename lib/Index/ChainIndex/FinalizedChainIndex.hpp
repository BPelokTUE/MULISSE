#ifndef INDEX_CHAININDEX_FINALIZEDCHAININDEX_HPP
#define INDEX_CHAININDEX_FINALIZEDCHAININDEX_HPP

#include "Index/FinalizedIndex.hpp"
#include "Index/Traits/FinalizedTraits.hpp"

/**
 * @brief Set of finalized indexes, intended to be used in a chain, with the approximate indexes being used first and
 * the exact index at the end.
 * @tparam FTag The traits of the entries in the index
 */
template <typename FTag>
    requires ValidEntryTraitsTag<FTag>
class FinalizedChainIndex : public IFinalizedIndex<FTag> {
   public:
    FinalizedChainIndex() = default;

    FinalizedChainIndex(vec<uptr<IFinalizedIndex<FTag>>> approx_indexes, uptr<IFinalizedIndex<FTag>> exact_index)
        : m_approx_indexes(std::move(approx_indexes)), m_exact_index(std::move(exact_index)) {}

    void save(const str &out_file, ArchiveType ar_type) override {
        for (uint approx_ind = 0; approx_ind < m_approx_indexes.size(); ++approx_ind)
            m_approx_indexes[approx_ind]->save(get_index_file_path(out_file, false, ar_type, approx_ind), ar_type);
        m_exact_index->save(get_index_file_path(out_file, true, ar_type), ar_type);
    }

    void load(const str &in_file, ArchiveType ar_type) override {
        for (uint approx_ind = 0; approx_ind < m_approx_indexes.size(); ++approx_ind)
            m_approx_indexes[approx_ind]->load(get_index_file_path(in_file, false, ar_type, approx_ind), ar_type);
        m_exact_index->load(get_index_file_path(in_file, true, ar_type), ar_type);
    }

    size_t get_size_on_disk(const str &index_file, const ArchiveType ar_type) const override {
        size_t size = 0;
        for (uint approx_ind = 0; approx_ind < m_approx_indexes.size(); ++approx_ind)
            size += m_approx_indexes[approx_ind]->get_size_on_disk(
                get_index_file_path(index_file, false, ar_type, approx_ind));
        size += m_exact_index->get_size_on_disk(get_index_file_path(index_file, true, ar_type));
        return size;
    }

    IFinalizedIndex<FTag> *release_approx_index(uint index) {
        assert(index < m_approx_indexes.size());
        return m_approx_indexes[index].release();
    }

    IFinalizedIndex<FTag> *release_exact_index() { return m_exact_index.release(); }

   private:
    vec<uptr<IFinalizedIndex<FTag>>> m_approx_indexes;
    uptr<IFinalizedIndex<FTag>> m_exact_index;

    /**
     * @brief Get the file path for an index file
     * @param path_base The base path of the file
     * @param exact Whether the index is exact or approximate
     * @param ar_type The file format of the index
     * @param approx_ind The index of the approximate index, if applicable
     * @return The file path for the index
     */
    str get_index_file_path(const str &path_base, bool exact, ArchiveType ar_type, uint approx_ind = 0) const {
        auto [base, extension] = get_file_base_and_extension(path_base);
        extension = extension.empty() ? get_archive_extension(ar_type) : extension;
        return base + (exact ? "_exact" : "_approx_" + std::to_string(approx_ind)) + extension;
    }
};

#endif  // INDEX_CHAININDEX_FINALIZEDCHAININDEX_HPP

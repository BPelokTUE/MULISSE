#ifndef INDEX_CHAININDEX_FINALIZEDCHAININDEX_HPP
#define INDEX_CHAININDEX_FINALIZEDCHAININDEX_HPP

#include "Index/FinalizedIndex.hpp"
#include "Util/Types/Pointers.hpp"

/**
 * @brief Set of finalized indexes, intended to be used in a chain, with the approximate indexes being used first and
 * the exact index at the end.
 * @tparam FTag The traits of the entries in the index
 */
template <typename FTag>
class FinalizedChainIndex : public IFinalizedIndex<FTag> {
   public:
    FinalizedChainIndex() = default;

    FinalizedChainIndex(vec<uptr<IFinalizedIndex<FTag>>> approx_indexes, uptr<IFinalizedIndex<FTag>> exact_index);

    void save(const str &out_file, ArchiveType ar_type) override;

    void load(const str &in_file, ArchiveType ar_type) override;

    size_t get_size_on_disk(const str &index_file, const ArchiveType ar_type) const override;

    IFinalizedIndex<FTag> *release_approx_index(uint index);

    IFinalizedIndex<FTag> *release_exact_index();

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

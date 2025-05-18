#ifndef INDEX_FINALIZEDINDEX_HPP
#define INDEX_FINALIZEDINDEX_HPP

#include <filesystem>

#include "Enums/ArchiveType.hpp"
#include "Index/Traits/EntryTags.hpp"
#include "Util/Types/Containers.hpp"

namespace fs = std::filesystem;

/**
 * @brief Interface for finalized indexes
 * @tparam T Traits of the entries in the index
 * */
template <typename T>
    requires ValidEntryTraitsTag<T>
class IFinalizedIndex {
   public:
    virtual ~IFinalizedIndex() = default;

    /**
     * @brief Save the index into a file
     * @param out_file Path to the output file
     * @param ar_type Archive type
     */
    virtual void save(const str &out_file, ArchiveType ar_type) = 0;

    /**
     * @brief Load the index from a file
     * @param in_file Path to the input file
     * @param ar_type Archive type
     */
    virtual void load(const str &in_file, ArchiveType ar_type) = 0;

    /**
     * @brief Get the size on disk in bytes of the saved index
     * @param index_file Path to the index file
     * @param ar_type Archive type
     * @return The size on disk in bytes
     */
    virtual size_t get_size_on_disk(const str &index_file, const ArchiveType ar_type = BINARY) const {
        str index_file_w_ext = add_archive_extension(index_file, ar_type);
        return fs::exists(index_file_w_ext) ? fs::file_size(index_file_w_ext) : 0;
    }
};

#endif  // INDEX_FINALIZEDINDEX_HPP

#ifndef INDEX_FINALIZEDINDEX_HPP
#define INDEX_FINALIZEDINDEX_HPP

#include <filesystem>

#include "Enums/ArchiveType.hpp"
#include "Index/Traits/EntryTags.hpp"
#include "Util/Artefacts/Artifact.hpp"
#include "Util/Types/String.hpp"

namespace fs = std::filesystem;

/**
 * @brief Interface for finalized indexes
 * @tparam T Traits of the entries in the index
 * */
template <typename FTag>
class IFinalizedIndex : public IArtifact {
   public:
    virtual ~IFinalizedIndex() = default;

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

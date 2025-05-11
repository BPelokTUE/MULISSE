#ifndef ENUMS_ARCHIVETYPE_HPP
#define ENUMS_ARCHIVETYPE_HPP

#include "Util/HelperFuncs/Enums.hpp"
#include "Util/HelperFuncs/Path.hpp"

/** @brief Enumeration type for the cereal archives */
enum ArchiveType { BINARY, JSON, NONE };

DEFINE_ENUM_CONSTS_NO_EXTRA(ArchiveType, ARCHIVE_TYPE, false);

/**
 * @brief Get the extension for a given archive type
 * @param ar_type The archive type
 * @return The extension for the archive type
 */
inline str get_archive_extension(ArchiveType ar_type) {
    switch (ar_type) {
        case BINARY:
            return ".bin";
        case JSON:
            return ".json";
        default:
            return "";
    }
}

/**
 * @brief Add the extension for a given archive type to a file name, if not already present
 * @param file_name The file name
 * @return The file name with the extension added, if not already present
 */
inline str add_archive_extension(const str &file_name, ArchiveType ar_type) {
    auto [base, extension] = get_file_base_and_extension(file_name);
    return base + (extension.empty() ? get_archive_extension(ar_type) : extension);
}

#endif  // ENUMS_ARCHIVETYPE_HPP

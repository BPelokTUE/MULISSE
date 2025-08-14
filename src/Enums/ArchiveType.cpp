#include "Enums/ArchiveType.hpp"

#include "Util/HelperFuncs/Path.hpp"

str get_archive_extension(ArchiveType ar_type) {
    switch (ar_type) {
        case ArchiveType::BINARY:
            return ".bin";
        case ArchiveType::JSON:
            return ".json";
        default:
            return "";
    }
}

str add_archive_extension(const str &file_name, ArchiveType ar_type) {
    auto [base, extension] = get_file_base_and_extension(file_name);
    return base + (extension.empty() ? get_archive_extension(ar_type) : extension);
}

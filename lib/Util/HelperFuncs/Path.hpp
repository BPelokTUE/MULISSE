#ifndef UTIL_HELPERFUNCS_PATH_HPP
#define UTIL_HELPERFUNCS_PATH_HPP

#include <filesystem>
#include <sstream>

#include "Util/Types/Containers.hpp"

namespace fs = std::filesystem;

/**
 * @brief Get the size of a dataset; TODO: this should be in `RunSettings`
 * @param dataset_path Path to the dataset
 * @return The size of the dataset
 */
inline size_t get_dataset_size(const str dataset_path) {
    try {
        return static_cast<size_t>(fs::file_size(dataset_path));
    } catch (const fs::filesystem_error& e) {
        throw std::runtime_error("Failed to get file size: " + std::string(e.what()));
    }
}

/**
 * @brief Get file base and extension
 * @param file_path Path to the file
 * @return The base name and extension of the file
 */
inline std::pair<str, str> get_file_base_and_extension(const str file_path) {
    auto path = fs::path(file_path);
    str dir_path = path.parent_path().string();
    if (!fs::exists(dir_path)) {
        throw std::runtime_error("Directory does not exist: " + dir_path);
    }
    str file_name = path.filename().string();
    auto dot_pos = file_name.find_last_of('.');

    str extension = (dot_pos == str::npos) ? "" : file_name.substr(dot_pos);
    file_name = (dot_pos == str::npos) ? file_name : file_name.substr(0, dot_pos);
    str base = (fs::canonical(dir_path) / file_name).string();

    return {base, extension};
}

#endif  // UTIL_HELPERFUNCS_PATH_HPP

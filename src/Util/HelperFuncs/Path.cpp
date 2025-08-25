#include "Util/HelperFuncs/Path.hpp"

#include <filesystem>
#include <fstream>

#include "Util/HelperFuncs/Errors.hpp"

namespace fs = std::filesystem;

// std::pair<str, str> get_file_base_and_extension(const str& file_path) {
//     auto path = fs::path(file_path);
//     str dir_path = path.parent_path().string();
//     if (!fs::exists(dir_path)) {
//         throw std::runtime_error("Directory does not exist: " + dir_path);
//     }
//     str file_name = path.filename().string();
//     auto dot_pos = file_name.find_last_of('.');

//     str extension = (dot_pos == str::npos) ? "" : file_name.substr(dot_pos);
//     file_name = (dot_pos == str::npos) ? file_name : file_name.substr(0, dot_pos);
//     str base = (fs::canonical(dir_path) / file_name).string();

//     return {base, extension};
// }

std::pair<str, str> get_file_base_and_extension(const str& file_path) {
    auto dot_pos = file_path.find_last_of('.');
    return {file_path.substr(0, dot_pos), file_path.substr(dot_pos)};
}

str append_to_base(const str& file_name, const str& to_append) {
    auto [base, extension] = get_file_base_and_extension(file_name);
    return base + to_append + extension;
}

void directory_exists(const str& dir_path) {
    if (!fs::exists(dir_path)) throw get_not_exist_error(dir_path);
    if (!fs::is_directory(dir_path)) throw get_not_a_directory_error(dir_path);
}

void check_file_is_readable(const str& file_path) {
    if (!fs::exists(file_path)) throw get_not_exist_error(file_path);
    if (!fs::is_regular_file(file_path)) throw get_not_a_regular_file_error(file_path);
    std::ifstream infile(file_path);
    if (!infile.is_open()) throw get_not_readable_error(file_path);
}

void check_file_is_writable(const str& file_path) {
    auto dir_path = fs::path(file_path).parent_path();
    check_directory_exists(dir_path);
    {
        std::ofstream outfile(file_path);
        if (!outfile.is_open()) {
            fs::remove(file_path);
            throw get_not_writable_error(file_path);
        }
    }
    fs::remove(file_path);
}

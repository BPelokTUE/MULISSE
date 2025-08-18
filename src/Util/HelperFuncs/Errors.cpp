#include "Util/HelperFuncs/Errors.hpp"

#include <format>

#include "Util/Types/LengthRange.hpp"

std::runtime_error get_not_exist_error(const str &path) { return std::runtime_error("Path does not exist: " + path); }

std::runtime_error get_not_a_directory_error(const str &path) {
    return std::runtime_error("Path is not a directory: " + path);
}

std::runtime_error get_not_a_regular_file_error(const str &path) {
    return std::runtime_error("Path is not a regular file: " + path);
}

std::runtime_error get_not_readable_error(const str &path) {
    return std::runtime_error("File is not readable: " + path);
}

std::runtime_error get_not_writable_error(const str &path) {
    return std::runtime_error("File is not writable: " + path);
}

std::runtime_error get_required_missing_error(const str &option) {
    return std::runtime_error("Required option missing: --" + option);
}

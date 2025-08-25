#include "Util/HelperFuncs/Errors.hpp"

#include <format>

#include "Util/Types/LengthRange.hpp"

namespace errors {
std::runtime_error not_exist(const str &path) { return std::runtime_error("Path does not exist: " + path); }

std::runtime_error not_a_directory(const str &path) { return std::runtime_error("Path is not a directory: " + path); }

std::runtime_error not_a_regular_file(const str &path) {
    return std::runtime_error("Path is not a regular file: " + path);
}

std::runtime_error not_readable(const str &path) { return std::runtime_error("File is not readable: " + path); }

std::runtime_error not_writable(const str &path) { return std::runtime_error("File is not writable: " + path); }

std::runtime_error required_missing(const str &option) {
    return std::runtime_error("Required option missing: --" + option);
}

constexpr std::runtime_error input_stream_not_set() { return std::runtime_error("Input stream is not set."); }

constexpr std::runtime_error output_stream_not_set() { return std::runtime_error("Output stream is not set."); }

constexpr std::runtime_error source_dataset_not_set() { return std::runtime_error("Source dataset is not set."); }
}  // namespace errors

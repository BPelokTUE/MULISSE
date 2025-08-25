#ifndef UTIL_HELPERFUNCS_ERRORS_HPP
#define UTIL_HELPERFUNCS_ERRORS_HPP

#include <stdexcept>

#include "Util/Types/String.hpp"

struct LengthRange;

namespace errors {
std::runtime_error not_exist(const str &path);

std::runtime_error not_a_directory(const str &path);

std::runtime_error not_a_regular_file(const str &path);

std::runtime_error not_readable(const str &path);

std::runtime_error not_writable(const str &path);

std::runtime_error required_missing(const str &option);

constexpr std::runtime_error input_stream_not_set();

constexpr std::runtime_error output_stream_not_set();

constexpr std::runtime_error source_dataset_not_set();
}  // namespace errors

#endif  // UTIL_HELPERFUNCS_ERRORS_HPP

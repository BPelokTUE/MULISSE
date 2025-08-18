#ifndef UTIL_HELPERFUNCS_ERRORS_HPP
#define UTIL_HELPERFUNCS_ERRORS_HPP

#include <stdexcept>

#include "Util/Types/String.hpp"

struct LengthRange;

std::runtime_error get_not_exist_error(const str &path);

std::runtime_error get_not_a_directory_error(const str &path);

std::runtime_error get_not_a_regular_file_error(const str &path);

std::runtime_error get_not_readable_error(const str &path);

std::runtime_error get_not_writable_error(const str &path);

std::runtime_error get_required_missing_error(const str &option);

#endif  // UTIL_HELPERFUNCS_ERRORS_HPP

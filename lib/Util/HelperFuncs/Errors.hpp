#ifndef UTIL_HELPERFUNCS_ERRORS_HPP
#define UTIL_HELPERFUNCS_ERRORS_HPP

#include <stdexcept>

#include "Util/Types/Containers.hpp"

std::runtime_error get_not_exist_error(const str &path);

std::runtime_error get_not_a_directory_error(const str &path);

std::runtime_error get_not_a_regular_file_error(const str &path);

std::runtime_error get_not_readable_error(const str &path);

std::runtime_error get_not_writable_error(const str &path);

std::runtime_error get_required_missing_error(const str &option);

constexpr std::runtime_error get_l_min_gt_l_max_error();

#endif  // UTIL_HELPERFUNCS_ERRORS_HPP

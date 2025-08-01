#ifndef MODULES_VALIDATORS_HPP
#define MODULES_VALIDATORS_HPP

#include "CLI11/CLI11.hpp"
#include "Util/Types/Pointers.hpp"

namespace validators {
const CLI::Validator positive_int, non_negative_int, non_negative_real, positive_real, zero_to_one_fraction,
    directory_exists, file_is_readable, file_is_writable;

CLI::Validator get_ge_validator(uint min_value);

CLI::Validator get_le_validator(uint max_value);
};  // namespace validators

#endif  // MODULES_VALIDATORS_HPP

#ifndef MODULES_VALIDATORS_HPP
#define MODULES_VALIDATORS_HPP

#include "CLI11/CLI11.hpp"

CLI::Validator get_positive_int_validator();

CLI::Validator get_non_negative_real_validator();

CLI::Validator get_positive_real_validator();

CLI::Validator get_zero_to_one_fraction_validator();

#endif  // MODULES_VALIDATORS_HPP

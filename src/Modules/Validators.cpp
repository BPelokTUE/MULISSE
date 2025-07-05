#include "Modules/Validators.hpp"

#include "Util/HelperFuncs/Conversion.hpp"
#include "Util/Types/Containers.hpp"

CLI::Validator get_positive_int_validator() {
    return CLI::Validator(
        [](str &input) {
            try {
                uint value = U(std::stoul(input));
                if (value > 0) {
                    return "";
                } else {
                    return "Value must be greater than 0";
                }
            } catch (const std::exception &) {
                return "Could not convert";
            }
        },
        "POSITIVE_INTEGER", "Positive Integer");
}

CLI::Validator get_non_negative_real_validator() {
    return CLI::Validator(
        [](str &input) {
            try {
                double value = std::stod(input);
                if (value >= 0.0) {
                    return "";
                } else {
                    return "Value must be non-negative";
                }
            } catch (const std::exception &) {
                return "Could not convert";
            }
        },
        "NON_NEGATIVE_REAL", "Non-Negative Real");
}

CLI::Validator get_positive_real_validator() {
    return CLI::Validator(
        [](str &input) {
            try {
                double value = std::stod(input);
                if (value > 0.0) {
                    return "";
                } else {
                    return "Value must be greater than 0";
                }
            } catch (const std::exception &) {
                return "Could not convert";
            }
        },
        "POSITIVE_REAL", "Positive Real");
}

CLI::Validator get_zero_to_one_fraction_validator() {
    return CLI::Validator(
        [](str &input) {
            try {
                double value = std::stod(input);
                if (value >= 0.0 && value <= 1.0) {
                    return "";
                } else {
                    return "Value must be between 0 and 1";
                }
            } catch (const std::exception &) {
                return "Could not convert";
            }
        },
        "FRACTION", "Fraction");
}

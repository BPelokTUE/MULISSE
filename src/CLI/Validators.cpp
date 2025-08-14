#include "CLI/Validators.hpp"

#include "Util/HelperFuncs/Conversion.hpp"
#include "Util/HelperFuncs/Path.hpp"

namespace validators {
const CLI::Validator positive_int = CLI::Validator(
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

const CLI::Validator non_negative_int = CLI::Validator(
    [](str &input) {
        try {
            uint value = U(std::stoul(input));
            if (value >= 0) {
                return "";
            } else {
                return "Value must be greater than 0";
            }
        } catch (const std::exception &) {
            return "Could not convert";
        }
    },
    "NON_NEGATIVE_INT", "Non-Negative Integer");

const CLI::Validator non_negative_real = CLI::Validator(
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

const CLI::Validator positive_real = CLI::Validator(
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

const CLI::Validator zero_to_one_fraction = CLI::Validator(
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
    "FRACTION", "Fraction Between 0 and 1");

const CLI::Validator directory_exists = CLI::Validator(
    [](const std::string &path) {
        try {
            check_file_is_readable(path);
            return std::string();
        } catch (const std::exception &e) {
            return std::string(e.what());
        }
    },
    "DIRECTORY_EXISTS", "Directory Exists");

const CLI::Validator file_is_readable = CLI::Validator(
    [](const std::string &path) {
        try {
            check_file_is_readable(path);
            return std::string();
        } catch (const std::exception &e) {
            return std::string(e.what());
        }
    },
    "FILE_IS_READABLE", "File is Readable");

const CLI::Validator file_is_writable = CLI::Validator(
    [](const std::string &path) {
        try {
            check_file_is_writable(path);
            return std::string();
        } catch (const std::exception &e) {
            return std::string(e.what());
        }
    },
    "FILE_IS_WRITABLE", "File is Writable");

CLI::Validator get_ge_validator(uint min_value) {
    return CLI::Validator(
        [min_value](str &input) {
            try {
                uint value = U(std::stoul(input));
                if (value >= min_value) {
                    return std::string();
                } else {
                    return "Value must be greater than or equal to " + std::to_string(min_value);
                }
            } catch (const std::exception &) {
                return "Could not convert";
            }
        },
        "GE_" + std::to_string(min_value), "Greater than or Equal to " + std::to_string(min_value));
}

CLI::Validator get_le_validator(uint max_value) {
    return CLI::Validator(
        [max_value](str &input) {
            try {
                uint value = U(std::stoul(input));
                if (value <= max_value) {
                    return std::string();
                } else {
                    return "Value must be less than or equal to " + std::to_string(max_value);
                }
            } catch (const std::exception &) {
                return "Could not convert";
            }
        },
        "LE_" + std::to_string(max_value), "Less than or Equal to " + std::to_string(max_value));

}  // namespace validators

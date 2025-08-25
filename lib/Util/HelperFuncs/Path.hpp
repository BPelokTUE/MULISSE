#ifndef UTIL_HELPERFUNCS_PATH_HPP
#define UTIL_HELPERFUNCS_PATH_HPP

#include <filesystem>
#include <sstream>

#include "Util/Types/String.hpp"

namespace fs = std::filesystem;

/**
 * @brief Get file base and extension
 * @param file_path Path to the file
 * @return The base name and extension of the file
 */
std::pair<str, str> get_file_base_and_extension(const str& file_path);

/**
 * @brief Append a string to the base of a file path
 * @param file_name Name of the file
 * @param to_append String to append to the base
 * @return The new file path with the appended string
 */
str append_to_base(const str& file_name, const str& to_append);

/**
 * @brief Check if directory exists, throw error otherwise
 * @param dir_path Path to the directory
 * @throw std::runtime_error if the directory does not exist
 */
void check_directory_exists(const str& dir_path);

/**
 * @brief Check if file can be read from, throw error otherwise
 * @param file_path Path to the file
 * @throw std::runtime_error if the file does not exist or cannot be read
 */
void check_file_is_readable(const str& file_path);

/**
 * @brief Check if file can be written to, throw error otherwise
 * @param file_path Path to the file
 * @throw std::runtime_error if the file does not exist or cannot be written to
 */
void check_file_is_writable(const str& file_path);

#endif  // UTIL_HELPERFUNCS_PATH_HPP

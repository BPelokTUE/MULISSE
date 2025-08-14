#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <filesystem>
#include <fstream>

#include "Util/Types/String.hpp"
#include "Util/Types/UMap.hpp"

using std::to_string;

namespace fs = std::filesystem;

// Enums for statistics

using TimePoint = std::chrono::time_point<std::chrono::high_resolution_clock>;

class Logger {
   public:
    virtual ~Logger() = default;

   protected:
    /**
     * @brief Determine the index of the new entry in the given file
     * @param file_path The path to the file
     * @return The index of the new entry
     */
    uint determine_index(const str &file_path);

    /**
     * @brief Create a file with the given header if it does not exist
     * @param file_path The path to the file
     * @param header The header of the file
     */
    void file_setup(const str &file_path, const vec<str> &header);

    /**
     * @brief Write a row into the given file stream
     * @tparam C The type of the column enums
     * @param file_path The path to the file
     * @param enum_to_val A map from the column enums to values
     * @param columns A vector defining the order of the columns
     */
    template <typename C>
    void write_row(const str &file_path, const umap<C, str> &enum_to_val, const vec<C> &columns) {
#ifndef DISABLE_LOGGING
        std::ofstream ofs(file_path, std::ios::app);

        ofs << ROW_SEP;
        for (uint i = 0; i < columns.size(); ++i) {
            C col = columns[i];
            if (enum_to_val.find(col) != enum_to_val.end()) {
                ofs << enum_to_val.at(col);
            } else {
                ofs << "";
            }
            if (i < columns.size() - 1) ofs << COL_SEP;
        }
#endif
    }

    /**
     * @brief Get the string representation of the given number, or emtpy string if the number is zero
     * @tparam T The type of the number
     * @param num The number
     * @return The string representation of the number, or empty string if the number is zero
     */
    template <typename T>
    static str format_num_param(T num) {
        return num == 0 ? "" : to_string(num);
    }

    template <typename T>
    str get_collection_str(const vec<T> &values) {
        str result_str = "";
        for (uint i = 0; i < values.size(); ++i) {
            if constexpr (std::is_same_v<T, str>) {
                result_str += values[i];
            } else {
                result_str += to_string(values[i]);
            }
            if (i < values.size() - 1) result_str += ITEM_SEP;
        }
        return result_str;
    }

    // Separators
    const char COL_SEP = ',', ROW_SEP = '\n', ITEM_SEP = ';';
};

#endif  // LOGGER_HPP

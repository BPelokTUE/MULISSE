#include "Util/Logging/Logger.hpp"

#include <fstream>

using std::to_string;

// Logger
uint Logger::determine_index(const str &file_path) {
    uint index = 0;
#ifndef DISABLE_LOGGING
    std::ifstream file_stream(file_path);
    str line;
    std::getline(file_stream, line);  // Skip the header
    while (std::getline(file_stream, line)) ++index;
#endif  // DISABLE_LOGGING
    return index;
}

void Logger::file_setup(const str &file_path, const vec<str> &header) {
#ifndef DISABLE_LOGGING
    // If the directory does not exist, create it
    std::filesystem::create_directories(std::filesystem::path(file_path).parent_path());

    if (std::filesystem::exists(file_path)) {
        std::ifstream file(file_path);
        if (file.peek() != std::ifstream::traits_type::eof()) {
            return;
        }
    }

    std::ofstream ofs(file_path);
    for (uint i = 0; i < header.size(); ++i) {
        ofs << header[i];
        if (i < header.size() - 1) ofs << COL_SEP;
    }
#endif  // DISABLE_LOGGING
}

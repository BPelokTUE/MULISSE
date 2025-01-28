#include "util.hpp"

#include <fstream>

size_t get_dataset_size(const str dataset_path) {
    std::ifstream data_stream(dataset_path, std::ios::binary | std::ios::ate);
    return data_stream.tellg();
}

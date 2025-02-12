#include <iostream>
#include <fstream>

#include "CLI11/CLI11.hpp"

#include "Util/typedefs.hpp"
#include "Search/iSax/iSaxEnvelopeFinalizedIndex.hpp"

int main(int argc, char **argv) {
    // Parse arguments
    CLI::App app{"Analyze MULISSE envelopes"};
    vec<str> index_paths;
    str index_format_str = ACCEPTED_ARCHIVE_TYPE_STRS[0];
    app.add_option("-i,--indexes", index_paths, "Index file path")->required();
    app.add_option("-f,--format", index_format_str, "Index format")
        ->capture_default_str()
        ->check(CLI::IsMember(ACCEPTED_ARCHIVE_TYPE_STRS));
    CLI11_PARSE(app, argc, argv);

    std::cout << "Showing statistics for the symbol ranges of the first layers of indexes. The symbol range for a "
                 "segment is equal to (max_symbol - min_symbol + 1) / alphabet_size.\n\n";

    for (auto &index_path : index_paths) {
        // Load index
        auto index = iSaxEnvelopeFinalizedIndex();
        std::ifstream ifs(index_path, std::ios::binary);
        index.load(ifs, STR_TO_ARCHIVE_TYPE.at(index_format_str));

        // Analyze envelopes
        auto &min_symbols = index.get_first_layer_min_symbols();
        auto &max_symbols = index.get_first_layer_max_symbols();
        SaxNumBitsT num_bits = index.get_first_layer_num_bits();
        float po2 = 1 << num_bits;

        MtsNumChannelsT num_channels = min_symbols[0].size();
        SaxSegIndT num_segments = min_symbols[0][0].size();
        vec<float> ranges;

        for (uint i = 0; i < min_symbols.size(); ++i) {
            for (MtsNumChannelsT c = 0; c < num_channels; ++c) {
                for (SaxSegIndT s = 0; s < num_segments; ++s) {
                    float diff = max_symbols[i][c][s] - min_symbols[i][c][s] + 1;
                    ranges.push_back(diff / po2);
                }
            }
        }

        // Print diff statistics: mean, median, min, max
        float mean = 0, min = ranges[0], max = ranges[0], median = 0;
        uint median_cnt = 0;
        umap<float, uint> counts;

        for (auto &range : ranges) {
            mean += range;
            if (range < min) min = range;
            if (range > max) max = range;
            if (++counts[range] > median_cnt) {
                median = range;
                median_cnt = counts[range];
            }
        }

        std::cout << "Statistics for " << index_path << ":\n"
                  << "Mean: " << mean / ranges.size() << '\n'
                  << "Min: " << min << '\n'
                  << "Max: " << max << '\n'
                  << "Median: " << median << "\n\n";
    }

    return 0;
}

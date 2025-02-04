#include "Modules/Searching.hpp"

#include "Search/iSax/iSaxEnvelopeFinalizedIndex.hpp"

uptr<IEnvelopeFinalizedIndex> load_index(IndexType index_type, const str &index_path, ArchiveType index_format) {
    std::ifstream index_stream(index_path, std::ios::binary);
    uptr<IEnvelopeFinalizedIndex> index;

    switch (index_type) {
        case ISAX_ENVELOPE:
            index = std::make_unique<iSaxEnvelopeFinalizedIndex>();
            break;
        default:
            return nullptr;
    }
    index->load(index_stream, index_format);
    return index;
}

/**
 * @brief Execute similarity search
 *
 * @param opts Options for searching
 */
int search(const SearchOptions &opts) {
    uptr<IEnvelopeFinalizedIndex> index = load_index(opts.index_type, opts.index_path, opts.index_format);

    if (index == nullptr) {
        std::cout << "Index type not implemented\n";
        return 1;
    }

    std::ifstream query_stream(opts.query_path);
    std::ofstream result_stream(opts.results_path);
    result_stream << std::fixed << std::setprecision(6);

    MtsNumChannelsT num_channels = index->get_num_channels();
    vec<vec<float>> query(num_channels);

    size_t query_count = 0;
    for (MtsNumChannelsT c = 0; !query_stream.eof(); c = (c + 1) % num_channels) {
        str line;
        std::getline(query_stream, line);
        std::istringstream iss(line);
        float value, sum = 0, sq_sum = 0;

        query[c].clear();
        while (iss >> value) {
            query[c].push_back(value);
            sum += value;
            sq_sum += value * value;
        }
        if (query[c].size() > 0) {
            float mu = sum / query[c].size(), sigma = sq_sum / query[c].size() - mu * mu;
            for (size_t i = 0; i < query[c].size(); ++i) query[c][i] = (query[c][i] - mu) / std::sqrt(sigma);
        }

        if (c == num_channels - 1) {
            opts.result_set->clear();
            vec<SearchResult> results = index->search(query, opts);
            result_stream << "Results for query " << ++query_count << ":\n";
            for (auto &result : results)
                result_stream << "Position: " << result.file_position << "; Distance: " << result.distance << '\n';
            result_stream << '\n';
        }
    }

    return 0;
}

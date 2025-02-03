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

    MtsNumChannelsT num_channels = index->get_num_channels();
    vec<vec<float>> query(num_channels);

    MtsNumChannelsT c = 0;
    size_t query_count = 0;

    for (str line; std::getline(query_stream, line); c = (c + 1) % num_channels) {
        std::istringstream iss(line);
        float value;
        while (iss >> value) {
            query[c].push_back(value);
        }

        if (c == num_channels - 1) {
            vec<SearchResult> results = index->search(query, opts);
            result_stream << "Results for query " << ++query_count << ":\n";
            for (auto &result : results) result_stream << result.file_position << ' ' << result.distance << '\n';
            result_stream << '\n';
        }
    }

    return 0;
}

#include "Modules/Searching.hpp"

#include "Util/RunSettings.hpp"
#include "Search/iSax/iSaxEnvelopeFinalizedIndex.hpp"
#include "Search/SequentialScan.hpp"

uptr<ISearchMethod> load_method(const SearchOptions &opts) {
    switch (opts.search_method_type) {
        case ISAX_ENVELOPE: {
            if (opts.index_path.empty()) {
                std::cerr << "No index path provided for search with iSAX envelope index\n";
                return nullptr;
            }

            std::ifstream index_stream(opts.index_path, std::ios::binary);
            auto index = std::make_unique<iSaxEnvelopeFinalizedIndex>();
            static_cast<IEnvelopeFinalizedIndex *>(index.get())->load(index_stream, opts.index_format);

            return index;
        }
        case SEQUENTIAL_SCAN:
            return std::make_unique<SequentialScan>();
    }
}

/**
 * @brief Execute similarity search
 *
 * @param opts Options for searching
 */
int search(const SearchOptions &opts) {
    uptr<ISearchMethod> method = load_method(opts);

    if (!method) return 1;

    auto &RS = RunSettings::get_instance();
    std::ifstream query_stream(opts.query_path);
    std::ofstream result_stream(opts.results_path);
    result_stream << std::fixed << std::setprecision(6);

    MtsNumChannelsT num_channels = RunSettings::get_instance().get_dataset_props().num_channels;
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
            auto [mu, sigma] = calculate_mu_and_sigma(sum, sq_sum, query[c].size());
            for (size_t i = 0; i < query[c].size(); ++i) query[c][i] = (query[c][i] - mu) / sigma;
        }

        if (c == num_channels - 1) {
            opts.result_set->clear();
            if (RS.ffts_supported()) RS.reset_query_ffts();

            vec<SearchResult> results = method->search(query, opts);
            result_stream << "Results for query " << ++query_count << std::endl;
            for (auto &result : results)
                result_stream << "Distance: " << std::sqrt(result.distance) << ", Location: " << result.file_position
                              << '\n';
            result_stream << '\n';
        }
    }

    return 0;
}

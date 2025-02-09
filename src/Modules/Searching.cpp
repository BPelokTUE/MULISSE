#include "Modules/Searching.hpp"
#include "Search/Options/SearchOptions.hpp"
#include "Util/typedefs.hpp"
#include "Util/RunSettings.hpp"
#include "Util/Logger.hpp"
#include "Search/iSax/iSaxEnvelopeFinalizedIndex.hpp"
#include "Search/SequentialScan.hpp"

uptr<ISearchMethod> load_method(const SearchOptions &opts) {
    switch (opts.search_method_type) {
        case ISAX_ENVELOPE: {
            str index_path = RunSettings::get_instance().get_index_path();
            if (index_path.empty()) {
                std::cerr << "No index path provided for search with iSAX envelope index\n";
                return nullptr;
            }

            std::ifstream index_stream(index_path, std::ios::binary);
            auto index = std::make_unique<iSaxEnvelopeFinalizedIndex>();
            static_cast<IEnvelopeFinalizedIndex *>(index.get())->load(index_stream, opts.index_format);

            return index;
        }
        case SEQUENTIAL_SCAN:
            return std::make_unique<SequentialScan>();
    }
    return nullptr;
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
    std::ifstream dataset_ifs(RS.get_dataset_path(), std::ios::binary);
    std::ifstream query_ifs(RS.get_query_path());

    QueryLogger::initialize(opts);
    auto &logger = QueryLogger::get_instance();

    MtsNumChannelsT num_channels = RunSettings::get_instance().get_dataset_props().num_channels;
    vec<vec<float>> query(num_channels);

    size_t query_count = 0;
    for (MtsNumChannelsT c = 0; !query_ifs.eof(); c = (c + 1) % num_channels) {
        str line;
        std::getline(query_ifs, line);
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
            logger.reset_entry();
            logger.set_number_col(QC::QUERY_ID, query_count++);
            logger.log_query(query);

            dataset_ifs.seekg(0);
            opts.result_set->clear();
            if (RS.ffts_supported()) RS.reset_query_ffts();

            logger.start_timer(QC::TOTAL_TIME_S);
            vec<SearchResult> results = method->search(query, opts, dataset_ifs);
            logger.stop_timer(QC::TOTAL_TIME_S);

            logger.log_results(results);

            logger.write_entry();
        }
    }

    return 0;
}

#include <iostream>
#include <string>
#include <fstream>

#include "CLI11/CLI11.hpp"

#include "Modules/RandomWalk.hpp"
#include "Modules/QueryGen.hpp"
#include "Modules/Indexing.hpp"
#include "Modules/Searching.hpp"

using std::string, std::cout;

int main(int argc, char **argv) {
    CLI::App app{"Run ULISSE-MTS"};

    // Add subcommands
    auto ds_subcommand = app.add_subcommand("create_ds", "Create random walk dataset");
    auto qs_subcommand = app.add_subcommand("create_qs", "Create queries from dataset");
    auto index_subcommand = app.add_subcommand("index", "Construct ULISSE MTS index");
    auto search_subcommand = app.add_subcommand("search", "Search using ULISSE MTS");
    app.require_subcommand(1);

    // Define custom validators
    auto positive_int = CLI::Validator(
        [](std::string &input) {
            try {
                unsigned value = std::stoi(input);
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

    auto positive_float = CLI::Validator(
        [](std::string &input) {
            try {
                float value = std::stof(input);
                if (value > 0.0) {
                    return "";
                } else {
                    return "Value must be greater than 0";
                }
            } catch (const std::exception &) {
                return "Could not convert";
            }
        },
        "POSITIVE_FLOAT", "Positive Float");

    // Add arguments
    /*
                | create_ds | create_query | index | search |
    dataset     |     X     |       X      |   X   |   X    |
    seed        |     X     |       X      |       |        |
    noise       |     X     |       X      |       |        |
    zero_start  |     X     |              |       |        |
    n           |     X     |              |       |        |
    m           |     X     |       X      |   X   |        |
    c           |     X     |       X      |   X   |        |
    Q           |           |       X      |       |        |
    lengths     |           |       X      |       |        |
    query_path  |           |       X      |       |   X    |
    index_type  |           |              |   X   |   X    |
    sps_type    |           |              |   X   |        |
    bps_type    |           |              |   X   |        |
    l_min       |           |              |   X   |        |
    l_max       |           |              |   X   |        |
    s           |           |              |   X   |        |
    pos_per_env |           |              |   X   |        |
    leaf_th     |           |              |   X   |        |
    index_path  |           |              |   X   |   X    |
    format      |           |              |   X   |   X    |
    approx/ex   |           |              |       |   X    |
    kNN/r-ran   |           |              |       |   X    |
    k(NN)       |           |              |       |   X    |
    r(range)    |           |              |       |   X    |
    normalize   |           |              |       |   X    |
    out         |           |              |       |   X    |
    */

    string dataset_path, query_path, index_path, results_path,
        index_type_str = INDEX_TYPE_STRS[0], split_strategy_str = ISAX_SPLIT_STRATEGY_STRS[0],
        breakpoint_strategy_str = ISAX_BREAKPOINT_STRATEGY_STRS[0], index_format_str = ARCHIVE_TYPE_STRS[0],
        search_type_str, distance_measure_str = DISTANCE_TYPE_STRS[0];
    float noise = 1.0;
    unsigned num_series, series_len, num_queries, l_min, l_max, segment_len, pos_per_env, knn_k = 1;
    DistanceT r_range_r = 1.0;
    int seed = 0;
    size_t leaf_capacity;
    vec<unsigned> lengths;
    MtsNumChannelsT num_channels;
    bool zero_start = false, unnormalized = false, approximate = false;

    // Options for creating dataset
    ds_subcommand->add_option("-d,--dataset", dataset_path, "Output dataset path")->required();
    ds_subcommand->add_option("--noise", noise, "Random walk standard deviation")->capture_default_str();
    ds_subcommand->add_flag("-z,--zero_start", zero_start, "Start the random walk from zero");
    ds_subcommand->add_option("-n,--num_series", num_series, "Number of series")->required()->check(positive_int);
    ds_subcommand->add_option("-m,--series_len", series_len, "Length of series")->required()->check(positive_int);
    ds_subcommand->add_option("-c,--num_channels", num_channels, "Number of channels")->required()->check(positive_int);
    ds_subcommand->add_option("-S,--seed", seed, "Random seed")->capture_default_str();

    // Options for creating queries
    qs_subcommand->add_option("-d,--dataset", dataset_path, "Dataset to use")->required()->check(CLI::ExistingFile);
    qs_subcommand->add_option("-q,--query", query_path, "Output query path")->required();
    qs_subcommand->add_option("--noise", noise, "Query noise")->capture_default_str();
    qs_subcommand->add_option("-m,--series_len", series_len, "Length of series")->required()->check(positive_int);
    qs_subcommand->add_option("-c,--num_channels", num_channels, "Number of channels")->required()->check(positive_int);
    qs_subcommand->add_option("-Q,--num_queries", num_queries, "Number of queries")->required()->check(positive_int);
    qs_subcommand->add_option("-l,--lengths", lengths, "Query lengths")->required()->check(positive_int);
    qs_subcommand->add_option("-S,--seed", seed, "Random seed")->capture_default_str();

    // Options for indexing
    index_subcommand->add_option("-i,--index", index_path, "Output index path")->required();
    index_subcommand->add_option("-d,--dataset", dataset_path, "Dataset to use")->required()->check(CLI::ExistingFile);
    index_subcommand->add_option("-m,--series_len", series_len, "Length of series")->required()->check(positive_int);
    index_subcommand->add_option("-c,--num_channels", num_channels, "Number of channels")
        ->required()
        ->check(positive_int);
    index_subcommand->add_option("-f,--format", index_format_str, "Index format")
        ->capture_default_str()
        ->check(CLI::IsMember(ARCHIVE_TYPE_STRS));
    index_subcommand->add_option("-t,--index_type", index_type_str, "Index type")
        ->capture_default_str()
        ->check(CLI::IsMember(INDEX_TYPE_STRS));
    index_subcommand->add_option("-S,--split_strategy", split_strategy_str, "Split strategy")
        ->capture_default_str()
        ->check(CLI::IsMember(ISAX_SPLIT_STRATEGY_STRS));
    index_subcommand->add_option("-B,--breakpoint_strategy", breakpoint_strategy_str, "Breakpoint strategy")
        ->capture_default_str()
        ->check(CLI::IsMember(ISAX_BREAKPOINT_STRATEGY_STRS));
    index_subcommand->add_option("-l,--l_min", l_min, "Minimum length of subsequences")
        ->required()
        ->check(positive_int);
    index_subcommand->add_option("-L,--l_max", l_max, "Maximum length of subsequences")
        ->required()
        ->check(positive_int);
    index_subcommand->add_option("-s,--segment_len", segment_len, "Segment length")->required()->check(positive_int);
    index_subcommand->add_option("-p,--pos_per_env", pos_per_env, "Positions per envelope")
        ->required()
        ->check(positive_int);
    index_subcommand->add_option("-C,--leaf_capacity", leaf_capacity, "Leaf capacity")->required()->check(positive_int);
    index_subcommand->add_flag("--raw", unnormalized, "Do not normalize");

    // Options for searching
    search_subcommand->add_option("-i,--index", index_path, "Index file path")->required()->check(CLI::ExistingFile);
    search_subcommand->add_option("-d,--dataset", dataset_path, "Dataset file path")
        ->required()
        ->check(CLI::ExistingFile);
    search_subcommand->add_option("-q,--query", query_path, "Query file path")->required()->check(CLI::ExistingFile);
    search_subcommand->add_option("-o,--out", results_path, "Output file path")->required();
    search_subcommand->add_option("-t,--index_type", index_type_str, "Index type")
        ->capture_default_str()
        ->check(CLI::IsMember(INDEX_TYPE_STRS));
    search_subcommand->add_option("-f,--format", index_format_str, "Index format")
        ->capture_default_str()
        ->check(CLI::IsMember(ARCHIVE_TYPE_STRS));
    search_subcommand->add_option("-D,--distance", distance_measure_str, "Distance measure")
        ->capture_default_str()
        ->check(CLI::IsMember(DISTANCE_TYPE_STRS));
    search_subcommand->add_flag("--approx", approximate, "Approximate search");
    search_subcommand->add_flag("--raw", unnormalized, "Do not normalize");
    //      Search type-specific options
    search_subcommand->add_option("-T,--search_type", search_type_str, "Search type")
        ->required()
        ->check(CLI::IsMember(SEARCH_TYPE_STRS));
    search_subcommand->add_option("-k,--k", knn_k, "Number of nearest neighbors for kNN")
        ->capture_default_str()
        ->check(positive_int);
    search_subcommand->add_option("-r,--range", r_range_r, "Range for range search")
        ->capture_default_str()
        ->check(positive_float);

    // For debugging (Clang 19 + Code LLDB + CLI11 don't like each other for some reason)
    dataset_path = "DATA/small/test.bin";
    index_path = "DATA/small/test_ind.bin";
    query_path = "DATA/query1.txt";
    results_path = "DATA/results_q1.txt";
    search_type_str = "knn";
    knn_k = 5;
    series_len = 4096;
    num_channels = 1;
    l_min = 256;
    l_max = 1024;
    segment_len = 64;
    pos_per_env = 16;
    leaf_capacity = 16;

    // Execute command
    CLI11_PARSE(app, argc, argv);

    if (ds_subcommand->parsed()) {
        create_random_walks(dataset_path, noise, zero_start, num_series, series_len, num_channels, seed);
    } else if (qs_subcommand->parsed()) {
        create_queries(dataset_path, query_path, noise, series_len, num_channels, num_queries, lengths, seed);
    } else if (index_subcommand->parsed()) {
        IndexType index_type = STR_TO_INDEX_TYPE.at(index_type_str);
        IIndexParams *index_params;
        switch (index_type) {
            case ISAX_ENVELOPE:
                index_params = new iSaxEnvelopeIndexParams{
                    pos_per_env,
                    segment_len,
                    1,  // first_layer_num_bits,
                    leaf_capacity,
                    STR_TO_ISAX_BREAKPOINT_STRATEGY.at(breakpoint_strategy_str),
                    STR_TO_ISAX_SPLIT_STRATEGY.at(split_strategy_str),
                    DEFAULT_NUM_BIT_LIMIT,
                };
                break;
            default:
                cout << "Index type \"" << index_type_str << "\" is not implemented\n";
                return 1;
        }
        IndexOptions index_options{
            .dataset_path = dataset_path,
            .index_path = index_path,
            .index_format = STR_TO_ARCHIVE_TYPE.at(index_format_str),
            .l_min = l_min,
            .l_max = l_max,
            .series_len = series_len,
            .num_channels = num_channels,
            .normalized = !unnormalized,
            .index_params = std::unique_ptr<IIndexParams>(index_params),
        };
        create_index(index_options);
    } else if (search_subcommand->parsed()) {
        SearchType search_type = STR_TO_SEARCH_TYPE.at(search_type_str);
        IDistanceMeasure *distance_measure;
        switch (STR_TO_DISTANCE_TYPE.at(distance_measure_str)) {
            case ED:
                distance_measure = new EuclideanDistance();
                break;
            default:
                cout << "Distance measure \"" << distance_measure_str << "\" is not implemented\n";
                return 1;
        }
        IResultSet *result_set;
        switch (search_type) {
            case KNN:
                result_set = new KnnResultSet(knn_k);
                break;
            case R_RANGE:
                result_set = new RRangeResultSet(r_range_r);
                break;
            default:
                cout << "Search type \"" << search_type_str << "\" is not implemented\n";
                return 1;
        }
        SearchOptions search_options = {
            .index_path = index_path,
            .dataset_path = dataset_path,
            .query_path = query_path,
            .results_path = results_path,
            .index_type = STR_TO_INDEX_TYPE.at(index_type_str),
            .index_format = STR_TO_ARCHIVE_TYPE.at(index_format_str),
            .exact = !approximate,
            .normalized = !unnormalized,
            .result_set = uptr<IResultSet>(result_set),
            .distance_measure = uptr<IDistanceMeasure>(distance_measure),
        };
        search(search_options);
    }

    return 0;
}

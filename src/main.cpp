#include <iostream>
#include <fstream>

#include "CLI11/CLI11.hpp"

#include "Modules/RandomWalk.hpp"
#include "Modules/CsvParsing.hpp"
#include "Modules/QueryGen.hpp"
#include "Modules/QueryStats.hpp"
#include "Modules/Indexing.hpp"
#include "Modules/CalcFfts.hpp"
#include "Modules/Searching.hpp"
#include "Util/constants.hpp"
#include "Util/typedefs.hpp"
#include "Util/RunSettings.hpp"

int main(int argc, char **argv) {
    CLI::App app{"Run MULISSE"};

    // Add subcommands
    auto rw_subcommand = app.add_subcommand(CMD_TYPE_TO_STR.at(CREATE_DS), "Create random walk dataset");
    auto csv_subcommand = app.add_subcommand(CMD_TYPE_TO_STR.at(PARSE_CSV), "Create dataset from CSV");
    auto qs_subcommand = app.add_subcommand(CMD_TYPE_TO_STR.at(CREATE_QS), "Create queries from dataset");
    auto q_stats_subcommand = app.add_subcommand(CMD_TYPE_TO_STR.at(CALC_Q_STATS), "Calculate query statistics");
    auto index_subcommand = app.add_subcommand(CMD_TYPE_TO_STR.at(INDEX), "Construct MULISSE index");
    auto ffts_subcommand = app.add_subcommand(CMD_TYPE_TO_STR.at(CALC_FFTS), "Calculate FFTs");
    auto search_subcommand = app.add_subcommand(CMD_TYPE_TO_STR.at(SEARCH), "Search using MULISSE");
    app.require_subcommand(1);

    // Define custom validators
    auto positive_int = CLI::Validator(
        [](str &input) {
            try {
                uint value = std::stoi(input);
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
        [](str &input) {
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
    str dataset_path, query_path, index_path, ffts_path,
        search_method_type_str = SEARCH_METHOD_TYPE_TO_STR.at(ISAX_ENVELOPE),
        split_strategy_str = ISAX_SPLIT_STRATEGY_TO_STR.at(ENTROPY_MAXIMIZING),
        breakpoint_strategy_str = ISAX_BREAKPOINT_STRATEGY_TO_STR.at(EQUIPROBABLE),
        index_format_str = ARCHIVE_TYPE_TO_STR.at(BINARY), search_type_str = SEARCH_TYPE_TO_STR.at(KNN),
        distance_measure_str = DISTANCE_TYPE_TO_STR.at(ED);
    vec<str> csv_paths;
    float step_sd = 1.0, noise = 0.1;
    SaxNumBitsT first_layer_num_bits = 1;
    uint num_series = 0, series_len, num_queries, l_min = 0, l_max = 0, segment_len, pos_per_env, knn_k = 1, low_sd_len;
    DistanceT r_range_r = 1.0;
    int seed;
    size_t leaf_capacity;
    vec<uint> exact_lengths = {};
    MtsNumChannelsT num_channels, used_channels = 0;
    vec<bool> channel_mask;
    bool zero_start = false, unnormalized = false, approximate = false, early_abandon = false;

    // Options for creating dataset
    rw_subcommand->add_option("-d,--dataset", dataset_path, "Output dataset path relative to `DATA`")->required();
    rw_subcommand->add_option("-s,--step_sd", step_sd, "Random walk step standard deviation")->capture_default_str();
    rw_subcommand->add_flag("-z,--zero_start", zero_start, "Start the random walk from zero");
    rw_subcommand->add_option("-n,--num_series", num_series, "Number of series")->required()->check(positive_int);
    rw_subcommand->add_option("-m,--series_len", series_len, "Length of series")->required()->check(positive_int);
    rw_subcommand->add_option("-c,--num_channels", num_channels, "Number of channels")->required()->check(positive_int);
    rw_subcommand->add_option("-S,--seed", seed, "Random seed")->capture_default_str();

    // Options for parsing csv
    csv_subcommand->add_option("-i,--input", csv_paths, "Input CSV file paths, in the order of channels")->required();
    csv_subcommand->add_option("-d,--dataset", dataset_path, "Output dataset path relative to `DATA`")->required();
    csv_subcommand->add_option("-n,--num_series", num_series, "Max number of series")->required()->check(positive_int);
    csv_subcommand
        ->add_option(
            "-l,--low_sd_len", low_sd_len,
            "Discard time series with any low standard deviation subsequence of this length. Pass 0 to disable.")
        ->required();
    csv_subcommand->add_option("-m,--series_len", series_len, "Length of series")->required()->check(positive_int);

    // Options for creating queries
    qs_subcommand->add_option("-d,--dataset", dataset_path, "Dataset to use")->required();
    qs_subcommand->add_option("-q,--query", query_path, "Output query path relative to `DATA`")->required();
    qs_subcommand->add_option("--noise", noise, "Query noise")->capture_default_str();
    qs_subcommand->add_option("-m,--series_len", series_len, "Length of series")->required()->check(positive_int);
    qs_subcommand->add_option("-c,--num_channels", num_channels, "Number of channels")->required()->check(positive_int);
    qs_subcommand->add_option("-Q,--num_queries", num_queries, "Number of queries")->required()->check(positive_int);
    qs_subcommand
        ->add_option("-e,--exact_lengths", exact_lengths,
                     "List of query lengths to generate. Is overriden by `--l_min` and `--l_max`.")
        ->capture_default_str()
        ->check(positive_int);
    qs_subcommand
        ->add_option(
            "-l,--l_min", l_min,
            "Minimum length of queries to generate. If passed `--l_max` is also required. Overrides `--exact_lengths`.")
        ->capture_default_str();
    qs_subcommand
        ->add_option(
            "-L,--l_max", l_max,
            "Maximum length of queries to generate. If passed `--l_min` is also required. Overrides `--exact_lengths`.")
        ->capture_default_str();
    qs_subcommand->add_option("-S,--seed", seed, "Random seed")->capture_default_str();
    qs_subcommand
        ->add_option("-u,--used_channels", used_channels,
                     "Number of channels to use for queries. 0 by default, meaning that the number of used "
                     "channels is selected randomly for each query.")
        ->capture_default_str();
    qs_subcommand
        ->add_option("-M,--channel_mask", channel_mask,
                     "Mask for which channels to use in the queries. Overrides "
                     "used_channels if provided.")
        ->capture_default_str();

    // Options for calculating query statistics
    q_stats_subcommand->add_option("-d,--dataset", dataset_path, "Dataset path relative to `DATA`")->required();
    q_stats_subcommand->add_option("-q,--query", query_path, "Query path relative to `DATA`")->required();
    q_stats_subcommand->add_option("-m,--series_len", series_len, "Length of series")->required()->check(positive_int);
    q_stats_subcommand->add_option("-c,--num_channels", num_channels, "Number of channels")
        ->required()
        ->check(positive_int);
    q_stats_subcommand->add_option("--noise", noise, "Query noise")->capture_default_str();
    q_stats_subcommand->add_flag("--raw", unnormalized, "Do not normalize");

    // Options for indexing
    index_subcommand->add_option("-i,--index", index_path, "Output index path relative to `DATA`")->required();
    index_subcommand->add_option("-d,--dataset", dataset_path, "Dataset path relative to `DATA`")->required();
    index_subcommand
        ->add_option("-F,--ffts", ffts_path,
                     "Path to save FFTs relative to `DATA`; if not provided, FFTs will not be calculated")
        ->capture_default_str();
    index_subcommand->add_option("-m,--series_len", series_len, "Length of series")->required()->check(positive_int);
    index_subcommand->add_option("-c,--num_channels", num_channels, "Number of channels")
        ->required()
        ->check(positive_int);
    index_subcommand->add_option("-f,--format", index_format_str, "Index format")
        ->capture_default_str()
        ->check(CLI::IsMember(ACCEPTED_ARCHIVE_TYPE_STRS));
    index_subcommand->add_option("-t,--index_type", search_method_type_str, "Index type")
        ->capture_default_str()
        ->check(CLI::IsMember(ACCEPTED_SEARCH_METHOD_TYPE_STRS));
    index_subcommand->add_option("-S,--split_strategy", split_strategy_str, "Split strategy")
        ->capture_default_str()
        ->check(CLI::IsMember(ACCEPTED_ISAX_SPLIT_STRATEGY_STRS));
    index_subcommand->add_option("-B,--breakpoint_strategy", breakpoint_strategy_str, "Breakpoint strategy")
        ->capture_default_str()
        ->check(CLI::IsMember(ACCEPTED_ISAX_BREAKPOINT_STRATEGY_STRS));
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
    index_subcommand->add_option("-b,--first_layer_bits", first_layer_num_bits, "Number of bits for first layer")
        ->check(positive_int)
        ->capture_default_str();

    // Options for calculating FFTs
    ffts_subcommand->add_option("-d,--dataset", dataset_path, "Dataset path relative to `DATA`")->required();
    ffts_subcommand->add_option("-F,--ffts", ffts_path, "Path to save FFTs relative to `DATA`")->required();
    ffts_subcommand->add_option("-m,--series_len", series_len, "Length of series")->required()->check(positive_int);
    ffts_subcommand->add_option("-c,--num_channels", num_channels, "Number of channels")
        ->required()
        ->check(positive_int);
    ffts_subcommand->add_flag("--raw", unnormalized, "Do not normalize");

    // Options for searching
    search_subcommand->add_option("-i,--index", index_path, "Index file path relative to `DATA`")
        ->capture_default_str();
    search_subcommand->add_option("-d,--dataset", dataset_path, "Dataset path relative to `DATA`")->required();
    search_subcommand->add_option("-q,--query", query_path, "Query file path relative to `DATA`")->required();
    search_subcommand
        ->add_option("-F,--ffts", ffts_path,
                     "Path to load FFTs from relative to `DATA`; if not provided, FFTs will not be loaded")
        ->capture_default_str();
    search_subcommand->add_option("-c,--num_channels", num_channels, "Number of channels")
        ->required()
        ->check(positive_int);
    search_subcommand->add_option("-m,--series_len", series_len, "Length of series")->required()->check(positive_int);
    search_subcommand->add_option("-t,--method_type", search_method_type_str, "Search method type")
        ->capture_default_str()
        ->check(CLI::IsMember(ACCEPTED_SEARCH_METHOD_TYPE_STRS));
    search_subcommand->add_option("-f,--format", index_format_str, "Index format")
        ->capture_default_str()
        ->check(CLI::IsMember(ACCEPTED_ARCHIVE_TYPE_STRS));
    search_subcommand->add_option("-D,--distance", distance_measure_str, "Distance measure")
        ->capture_default_str()
        ->check(CLI::IsMember(ACCEPTED_DISTANCE_TYPE_STRS));
    search_subcommand->add_flag("--early_abandon", early_abandon, "Use early abandoning");
    search_subcommand->add_flag("--approx", approximate, "Approximate search");
    search_subcommand->add_flag("--raw", unnormalized, "Do not normalize");
    //      Search type-specific options
    search_subcommand->add_option("-T,--search_type", search_type_str, "Search type")
        ->capture_default_str()
        ->check(CLI::IsMember(ACCEPTED_SEARCH_TYPE_STRS));
    search_subcommand->add_option("-k,--k", knn_k, "Number of nearest neighbors for kNN")
        ->capture_default_str()
        ->check(positive_int);
    search_subcommand->add_option("-r,--range", r_range_r, "Range for range search")
        ->capture_default_str()
        ->check(positive_float);

    // Parse arguments and initialize run settings
    CLI11_PARSE(app, argc, argv);
    CommandType command_type = STR_TO_CMD_TYPE.at(app.get_subcommands().front()->get_name());
    try {
        RunSettings::initialize(command_type, {dataset_path, num_channels, series_len, num_series},
                                {query_path, l_min, l_max}, pos_per_env, index_path, ffts_path);
    } catch (const std::exception &e) {
        std::cerr << "Error configuring run: " << e.what() << '\n';
        return 1;
    }

    // Execute subcommand
    if (command_type == CREATE_DS) {
        create_random_walks(step_sd, zero_start, seed);
    } else if (command_type == PARSE_CSV) {
        create_dataset_from_csv(csv_paths, num_series, low_sd_len);
    } else if (command_type == CREATE_QS) {
        create_queries(noise, num_queries, exact_lengths, l_min, l_max, used_channels, channel_mask, seed);
    } else if (command_type == CALC_Q_STATS) {
        calculate_query_stats(!unnormalized, noise);
    } else if (command_type == INDEX) {
        SearchMethodType index_type = STR_TO_SEARCH_METHOD_TYPE.at(search_method_type_str);
        IIndexParams *index_params;
        switch (index_type) {
            case ISAX_ENVELOPE:
                index_params = new iSaxEnvelopeIndexParams{
                    pos_per_env,
                    segment_len,
                    first_layer_num_bits,
                    leaf_capacity,
                    STR_TO_ISAX_BREAKPOINT_STRATEGY.at(breakpoint_strategy_str),
                    STR_TO_ISAX_SPLIT_STRATEGY.at(split_strategy_str),
                    DEFAULT_NUM_BIT_LIMIT,
                    true,  // min_num_bits_on_tie,
                };
                break;
            case ISAX:
                index_params = new iSaxIndexParams{
                    segment_len,
                    first_layer_num_bits,
                    leaf_capacity,
                    STR_TO_ISAX_BREAKPOINT_STRATEGY.at(breakpoint_strategy_str),
                    STR_TO_ISAX_SPLIT_STRATEGY.at(split_strategy_str),
                    DEFAULT_NUM_BIT_LIMIT,
                    true,  // min_num_bits_on_tie,
                };
            case SEQUENTIAL_SCAN:
                std::cerr << "Sequential scan does not require indexation\n";
                return 1;
            default:
                std::cerr << "Index type \"" << search_method_type_str << "\" is not implemented\n";
                return 1;
        }
        IndexOptions index_options{
            .index_format = STR_TO_ARCHIVE_TYPE.at(index_format_str),
            .l_min = l_min,
            .l_max = l_max,
            .series_len = series_len,
            .num_channels = num_channels,
            .normalized = !unnormalized,
            .index_params = std::unique_ptr<IIndexParams>(index_params),
        };
        create_index(index_options);
    } else if (command_type == CALC_FFTS) {
        calculate_ffts(!unnormalized);
    } else if (command_type == SEARCH) {
        SearchType search_type = STR_TO_SEARCH_TYPE.at(search_type_str);
        IDistanceMeasure *distance_measure;
        switch (STR_TO_DISTANCE_TYPE.at(distance_measure_str)) {
            case ED:
                distance_measure = new EuclideanDistance(!unnormalized, early_abandon);
                break;
            case MASS:
                distance_measure = new EuclideanDistanceWMass(!unnormalized);
                break;
            default:
                std::cerr << "Distance measure \"" << distance_measure_str << "\" is not implemented\n";
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
                std::cerr << "Search type \"" << search_type_str << "\" is not implemented\n";
                return 1;
        }
        SearchOptions search_options = {
            .search_method_type = STR_TO_SEARCH_METHOD_TYPE.at(search_method_type_str),
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

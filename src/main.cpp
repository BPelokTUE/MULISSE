#include <iostream>
#include <fstream>

#include "CLI11/CLI11.hpp"

#include "Modules/RandomWalk.hpp"
#include "Modules/CsvParsing.hpp"
#include "Modules/QueryGen.hpp"
#include "Modules/QueryStats.hpp"
#include "Modules/Indexing.hpp"
#include "Modules/IndexStats.hpp"
#include "Modules/CalcFfts.hpp"
#include "Modules/Searching.hpp"
#include "Search/DistanceMeasure.hpp"
#include "Search/ResultSet.hpp"
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
    auto i_stats_subcommand = app.add_subcommand(CMD_TYPE_TO_STR.at(CALC_I_STATS), "Calculate index statistics");
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

    auto positive_real = CLI::Validator(
        [](str &input) {
            try {
                double value = std::stod(input);
                if (value > 0.0) {
                    return "";
                } else {
                    return "Value must be greater than 0";
                }
            } catch (const std::exception &) {
                return "Could not convert";
            }
        },
        "POSITIVE_REAL", "Positive Real");

    // Add arguments
    str dataset_path, query_path, index_path, ffts_path,
        breakpoints_path = "", logs_path = "../LOGS",
        search_method_type_str = SEARCH_METHOD_TYPE_TO_STR.at(ISAX_ENVELOPE),
        split_strategy_str = ISAX_SPLIT_STRATEGY_TO_STR.at(ENTROPY_MAXIMIZING),
        breakpoint_strategy_str = ISAX_BREAKPOINT_STRATEGY_TO_STR.at(EQUIPROBABLE),
        index_format_str = ARCHIVE_TYPE_TO_STR.at(BINARY), search_type_str = SEARCH_TYPE_TO_STR.at(KNN),
        distance_measure_str = DISTANCE_TYPE_TO_STR.at(ED), inserter_type_str = ENTRY_INSERTER_TYPE_TO_STR.at(TOP_DOWN);
    vec<str> csv_paths;
    Real step_sd = 1.0, noise = 0.1;
    SaxNumBitsT first_layer_num_bits = 1, num_bits_limit = MAX_NUM_BITS_LIMIT;
    uint num_series = 0, series_len, num_queries, l_min = 0, l_max = 0, segment_len, pos_per_env = 0, knn_k = 1;
    Real r_range_r = 1.0;
    int seed;
    size_t leaf_capacity = 0, max_leaves_to_visit = 0;
    vec<uint> exact_lengths = {};
    MtsNumChannelsT num_channels, used_channels = 0;
    vec<bool> channel_mask;
    bool zero_start = false, unnormalized = false, approximate = false, early_abandon = false, sort_query = false,
         no_use_pq = false, adapt_index = false, prefer_first_in_em = false;

    // Options for creating dataset
    rw_subcommand->add_option("-d,--dataset", dataset_path, "Output dataset path relative to `DATA`")->required();
    rw_subcommand->add_option("-s,--step_sd", step_sd, "Random walk step standard deviation")->capture_default_str();
    rw_subcommand->add_flag("-z,--zero_start", zero_start, "Start the random walk from zero");
    rw_subcommand->add_option("-n,--num_series", num_series, "Number of series")->required()->check(positive_int);
    rw_subcommand->add_option("-m,--series_len", series_len, "Length of series")->required()->check(positive_int);
    rw_subcommand->add_option("-c,--num_channels", num_channels, "Number of channels")->required()->check(positive_int);
    rw_subcommand->add_option("-S,--seed", seed, "Random seed")->capture_default_str();
    rw_subcommand->add_option("--logs", logs_path, "Path to write logs to")->capture_default_str();

    // Options for parsing csv
    csv_subcommand->add_option("-i,--input", csv_paths, "Input CSV file paths, in the order of channels")->required();
    csv_subcommand->add_option("-d,--dataset", dataset_path, "Output dataset path relative to `DATA`")->required();
    csv_subcommand->add_option("-n,--num_series", num_series, "Max number of series")->required()->check(positive_int);
    csv_subcommand
        ->add_option("-l,--l_min", l_min,
                     "Minimum length of subsequences that will be queried for. Used for discarding series with "
                     "stagnant subsequences that would make normalization unstable")
        ->required();
    csv_subcommand
        ->add_option("-L,--l_max", l_max,
                     "Maximum length of subsequences that will be queried for. Used for discarding stagnant "
                     "subsequences that would make normalization unstable")
        ->required();
    csv_subcommand->add_option("-m,--series_len", series_len, "Length of series")->required()->check(positive_int);
    csv_subcommand->add_option("-S,--seed", seed, "Random seed")->capture_default_str();
    csv_subcommand->add_option("--logs", logs_path, "Path to write logs to")->capture_default_str();

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
    qs_subcommand->add_option("--logs", logs_path, "Path to write logs to")->capture_default_str();

    // Options for calculating query statistics
    q_stats_subcommand->add_option("-d,--dataset", dataset_path, "Dataset path relative to `DATA`")->required();
    q_stats_subcommand->add_option("-q,--query", query_path, "Query path relative to `DATA`")->required();
    q_stats_subcommand->add_option("-m,--series_len", series_len, "Length of series")->required()->check(positive_int);
    q_stats_subcommand->add_option("-c,--num_channels", num_channels, "Number of channels")
        ->required()
        ->check(positive_int);
    q_stats_subcommand->add_flag("--raw", unnormalized, "Do not normalize");
    q_stats_subcommand->add_option("--logs", logs_path, "Path to write logs to")->capture_default_str();

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
    index_subcommand->add_flag("--prefer_first_in_em", prefer_first_in_em,
                               "Prefer the first segment over the one with the minimum number of bits, in case of ties "
                               "in the split when using EntropyMaximizing strategy");
    index_subcommand->add_option("-B,--breakpoint_strategy", breakpoint_strategy_str, "Breakpoint strategy")
        ->capture_default_str()
        ->check(CLI::IsMember(ACCEPTED_ISAX_BREAKPOINT_STRATEGY_STRS));
    index_subcommand->add_option("--breakpoints", breakpoints_path, "Path to breakpoints file")->capture_default_str();
    index_subcommand->add_flag("--adapt", adapt_index, "Adapt the index properties to the dataset");
    index_subcommand->add_option("-l,--l_min", l_min, "Minimum length of subsequences")
        ->required()
        ->check(positive_int);
    index_subcommand->add_option("-L,--l_max", l_max, "Maximum length of subsequences")
        ->required()
        ->check(positive_int);
    index_subcommand->add_option("-s,--segment_len", segment_len, "Segment length")->required()->check(positive_int);
    index_subcommand->add_option("-p,--pos_per_env", pos_per_env, "Positions per envelope")
        ->capture_default_str()
        ->check(positive_int);
    index_subcommand->add_option("-C,--leaf_capacity", leaf_capacity, "Leaf capacity")
        ->capture_default_str()
        ->check(positive_int);
    index_subcommand->add_flag("--raw", unnormalized, "Do not normalize");
    index_subcommand->add_option("-b,--first_layer_bits", first_layer_num_bits, "Number of bits for first layer")
        ->check(positive_int)
        ->capture_default_str();
    index_subcommand->add_option("--num_bits_limit", num_bits_limit, "Maximum number of bits per segment")
        ->check(positive_int)
        ->capture_default_str();
    index_subcommand->add_option("-I,--inserter_type", inserter_type_str, "Entry inserter type")
        ->capture_default_str()
        ->check(CLI::IsMember(ACCEPTED_ENTRY_INSERTER_TYPE_STRS));
    index_subcommand->add_option("--logs", logs_path, "Path to write logs to")->capture_default_str();

    // Options for calculating index statistics
    i_stats_subcommand->add_option("-i,--index", index_path, "Index file path relative to `DATA`")->required();
    i_stats_subcommand->add_option("-c,--num_channels", num_channels, "Number of channels")
        ->required()
        ->check(positive_int);
    i_stats_subcommand->add_option("-f,--format", index_format_str, "Index format")
        ->capture_default_str()
        ->check(CLI::IsMember(ACCEPTED_ARCHIVE_TYPE_STRS));
    i_stats_subcommand->add_option("-t,--index_type", search_method_type_str, "Index type")
        ->capture_default_str()
        ->check(CLI::IsMember(ACCEPTED_SEARCH_METHOD_TYPE_STRS));
    i_stats_subcommand->add_option("--logs", logs_path, "Path to write logs to")->capture_default_str();

    // Options for calculating FFTs
    ffts_subcommand->add_option("-d,--dataset", dataset_path, "Dataset path relative to `DATA`")->required();
    ffts_subcommand->add_option("-F,--ffts", ffts_path, "Path to save FFTs relative to `DATA`")->required();
    ffts_subcommand->add_option("-m,--series_len", series_len, "Length of series")->required()->check(positive_int);
    ffts_subcommand->add_option("-c,--num_channels", num_channels, "Number of channels")
        ->required()
        ->check(positive_int);
    ffts_subcommand->add_flag("--raw", unnormalized, "Do not normalize");
    ffts_subcommand->add_option("--logs", logs_path, "Path to write logs to")->capture_default_str();

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
    search_subcommand->add_flag(
        "--sort_query", sort_query,
        "Sort data points of queries based on their absolute values. Only supported for Euclidean distance "
        "with early abandoning.");
    search_subcommand->add_flag("--no_pq,--no_priority_queue", no_use_pq,
                                "Do not use a priority queue for flat envelope index search");
    search_subcommand->add_flag("--approx", approximate, "Approximate search");
    search_subcommand
        ->add_option(
            "-M,--max_leaves_to_visit", max_leaves_to_visit,
            "Maximum number of leaves to visit if approximate search is used. Defaults to 0, indicating no max.")
        ->capture_default_str()
        ->check(positive_int);
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
        ->check(positive_real);
    search_subcommand->add_option("--logs", logs_path, "Path to write logs to")->capture_default_str();

    // Parse arguments and initialize run settings
    CLI11_PARSE(app, argc, argv);
    CommandType command_type = STR_TO_CMD_TYPE.at(app.get_subcommands().front()->get_name());
    SearchMethodType method_type = STR_TO_SEARCH_METHOD_TYPE.at(search_method_type_str);

    // Extra parsing; TODO: handle this with CLI11 if possible
    if (l_min > l_max) {
        std::cerr << "Minimum length must be less than or equal to maximum length\n";
        return 1;
    }
    if (used_channels > num_channels) {
        std::cerr << "Number of used channels must be less than or equal to the number of channels\n";
        return 1;
    }
    if (channel_mask.size() > 0 && channel_mask.size() != num_channels) {
        std::cerr << "Channel mask must have the same length as the number of channels\n";
        return 1;
    }
    if (command_type == INDEX) {
        if (method_type == ISAX || method_type == ISAX_ENVELOPE) {
            // Leaf capacity required
            if (leaf_capacity == 0) {
                std::cerr << "--leaf_capacity is required\n";
                return 1;
            }
            // Number of bits limit cannot be too high
            if (num_bits_limit > MAX_NUM_BITS_LIMIT) {
                std::cerr << "Maximum number of bits per segment must be less than or equal to " << MAX_NUM_BITS_LIMIT
                          << '\n';
                return 1;
            }
        }
        if (method_type == ISAX || method_type == ISAX_ENVELOPE || method_type == SAX_ENVELOPE) {
            // When using fixed breakpoints strategy, the breakpoints file must exist and must contain sufficient
            // breakpoints
            iSaxBreakpointStrategyType breakpoint_strategy_type =
                STR_TO_ISAX_BREAKPOINT_STRATEGY.at(breakpoint_strategy_str);

            if (breakpoint_strategy_type == FIXED) {
                std::ifstream breakpoints_ifs(breakpoints_path);
                if (!breakpoints_ifs) {
                    std::cerr << "Error: Could not open breakpoints file " << breakpoints_path << '\n';
                    return 1;
                }
                SaxSegIndT alphabet_size = 1;
                Real breakpoint;
                while (breakpoints_ifs >> breakpoint) ++alphabet_size;

                if (alphabet_size < (1 << num_bits_limit)) {
                    std::cerr << "The file " << breakpoints_path << " contains " << alphabet_size - 1
                              << " breakpoints, but " << (1 << num_bits_limit) - 1
                              << " are required. Provide a different file or lower the number of bits limit.\n";
                    return 1;
                }
            }
        }
        if ((method_type == ENVELOPE || method_type == SAX_ENVELOPE || method_type == ISAX_ENVELOPE) &&
            pos_per_env == 0) {
            std::cerr << "--pos_per_env is required\n";
            return 1;
        }
    } else if (command_type == SEARCH) {
        if (sort_query) {
            if (distance_measure_str != DISTANCE_TYPE_TO_STR.at(ED)) {
                std::cerr << "Sorting queries is only supported for Euclidean distance\n";
                return 1;
            }
            if (!early_abandon) {
                std::cerr << "Sorting queries is only supported with early abandoning\n";
                return 1;
            }
        }
    }

    // Initialize run settings
    try {
        RunSettings::initialize(command_type, {dataset_path, num_channels, series_len, num_series},
                                {query_path, l_min, l_max}, pos_per_env, index_path, ffts_path, method_type, logs_path);
    } catch (const std::exception &e) {
        std::cerr << "Error configuring run: " << e.what() << '\n';
        return 1;
    }

    // Execute subcommand
    switch (command_type) {
        case CREATE_DS: {
            return create_random_walks(step_sd, zero_start, seed);
        }
        case PARSE_CSV: {
            return create_dataset_from_csv(csv_paths, num_series, l_min, l_max, seed);
        }
        case CREATE_QS: {
            return create_queries({noise, num_queries, exact_lengths, l_min, l_max, used_channels, channel_mask, seed});
        }
        case CALC_Q_STATS: {
            return calculate_query_stats(!unnormalized);
        }
        case INDEX: {
            IIndexParams *index_params;
            switch (method_type) {
                case ISAX_ENVELOPE:
                    index_params = new iSaxEnvelopeIndexParams{
                        pos_per_env,
                        segment_len,
                        first_layer_num_bits,
                        leaf_capacity,
                        STR_TO_ISAX_BREAKPOINT_STRATEGY.at(breakpoint_strategy_str),
                        STR_TO_ISAX_SPLIT_STRATEGY.at(split_strategy_str),
                        num_bits_limit,
                        !prefer_first_in_em,
                        breakpoints_path,
                    };
                    break;
                case ISAX:
                    index_params = new iSaxIndexParams{
                        segment_len,
                        first_layer_num_bits,
                        leaf_capacity,
                        STR_TO_ISAX_BREAKPOINT_STRATEGY.at(breakpoint_strategy_str),
                        STR_TO_ISAX_SPLIT_STRATEGY.at(split_strategy_str),
                        num_bits_limit,
                        !prefer_first_in_em,
                        breakpoints_path,
                    };
                    break;
                case ENVELOPE:
                    index_params = new EnvelopeIndexParams{pos_per_env, segment_len};
                    break;
                case SAX_ENVELOPE: {
                    index_params = new SaxEnvelopeIndexParams{
                        pos_per_env,          segment_len,
                        first_layer_num_bits, STR_TO_ISAX_BREAKPOINT_STRATEGY.at(breakpoint_strategy_str),
                        !prefer_first_in_em,  breakpoints_path,
                    };
                    break;
                }
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
                .adapt = adapt_index,
                .inserter_type = STR_TO_ENTRY_INSERTER_TYPE.at(inserter_type_str),
                .index_params = std::unique_ptr<IIndexParams>(index_params),
            };
            return create_index(index_options);
        }
        case CALC_I_STATS: {
            return calculate_index_stats(method_type, STR_TO_ARCHIVE_TYPE.at(index_format_str));
        }
        case CALC_FFTS: {
            return calculate_ffts(!unnormalized);
        }
        case SEARCH: {
            SearchType search_type = STR_TO_SEARCH_TYPE.at(search_type_str);
            DistanceType distance_type = STR_TO_DISTANCE_TYPE.at(distance_measure_str);

            SearchOptions search_options = {
                .search_method_type = STR_TO_SEARCH_METHOD_TYPE.at(search_method_type_str),
                .index_format = STR_TO_ARCHIVE_TYPE.at(index_format_str),
                .search_type = search_type,
                .distance_type = distance_type,
                .knn_k = knn_k,
                .r_range_r = r_range_r,
                .exact = !approximate,
                .max_leaves_to_visit = max_leaves_to_visit,
                .normalized = !unnormalized,
                .use_early_abandoning = early_abandon,
                .sort_queries = sort_query,
                .use_priority_queue = !no_use_pq,
            };

            switch (distance_type) {
                case ED:
                    if (search_type == KNN) {
                        ResultSet<KNN> knn_result_set(knn_k);
                        if (sort_query) {
                            DistanceMeasure<KNN, ED, true> distance_measure(!unnormalized, early_abandon);
                            return search<KNN, ED, true>(search_options, knn_result_set, distance_measure);
                        } else {
                            DistanceMeasure<KNN, ED> distance_measure(!unnormalized, early_abandon);
                            return search<KNN, ED>(search_options, knn_result_set, distance_measure);
                        }
                    } else {  // search_type == R_RANGE
                        ResultSet<R_RANGE> result_set(r_range_r);
                        DistanceMeasure<R_RANGE, ED> distance_measure(!unnormalized, early_abandon);
                        return search<R_RANGE, ED>(search_options, result_set, distance_measure);
                    }
                case MASS:
                    if (STR_TO_SEARCH_TYPE.at(search_type_str) == KNN) {
                        ResultSet<KNN> knn_result_set(knn_k);
                        DistanceMeasure<KNN, MASS> distance_measure(!unnormalized);
                        return search<KNN, MASS>(search_options, knn_result_set, distance_measure);
                    } else {  // search_type == R_RANGE
                        ResultSet<R_RANGE> result_set(r_range_r);
                        DistanceMeasure<R_RANGE, MASS> distance_measure(!unnormalized);
                        return search<R_RANGE, MASS>(search_options, result_set, distance_measure);
                    }
                default:
                    std::cerr << "Distance measure \"" << distance_measure_str << "\" is not implemented\n";
                    return 1;
            }
        }
    }

    return 0;
}

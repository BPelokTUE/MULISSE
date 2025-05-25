#include <fstream>
#include <iostream>

#include "CLI11/CLI11.hpp"
#include "Index/EntryMerger/EntryMerger.hpp"
#include "Modules/CalcDatasetStats.hpp"
#include "Modules/CalcFfts.hpp"
#include "Modules/CalcIndexStats.hpp"
#include "Modules/CalcQueryStats.hpp"
#include "Modules/CsvParsing.hpp"
#include "Modules/Indexing/Indexing.hpp"
#include "Modules/QueryGen.hpp"
#include "Modules/RandomWalk.hpp"
#include "Modules/Searching.hpp"
#include "Search/DistanceMeasure/DistanceMeasure.hpp"
#include "Search/DistanceMeasure/EuclideanDistance.hpp"
#include "Search/DistanceMeasure/Mass.hpp"
#include "Search/Results/ResultSet.hpp"
#include "Serialization/SerializationRegistration.hpp"
#include "Util/Constants/Math.hpp"
#include "Util/Constants/Sax.hpp"
#include "Util/HelperFuncs/Containers.hpp"
#include "Util/RunSettings/RunSettings.hpp"
#include "Util/Types/Containers.hpp"
#include "Util/Types/Numbers.hpp"

int main(int argc, char **argv) {
    CLI::App app{"Run MULISSE"};

    // Add subcommands
    auto rw_subcommand = app.add_subcommand(CMD_TYPE_TO_STR.at(CREATE_DS), "Create random walk dataset");
    auto csv_subcommand = app.add_subcommand(CMD_TYPE_TO_STR.at(PARSE_CSV), "Create dataset from CSV");
    auto d_stats_subcommand = app.add_subcommand(CMD_TYPE_TO_STR.at(CALC_D_STATS), "Calculate dataset statistics");
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
                uint value = U(std::stoul(input));
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

    auto fraction = CLI::Validator(
        [](str &input) {
            try {
                double value = std::stod(input);
                if (value >= 0.0 && value <= 1.0) {
                    return "";
                } else {
                    return "Value must be between 0 and 1";
                }
            } catch (const std::exception &) {
                return "Could not convert";
            }
        },
        "FRACTION", "Fraction");

    // Add arguments
    str dataset_path, query_path, index_path, ffts_path,
        breakpoints_path = "", logs_path = "../LOGS",
        search_method_type_str = SEARCH_METHOD_TYPE_TO_STR.at(ISAX_ENVELOPE),
        lg_segmentation_strategy_str =
            LENGTH_GROUP_SEGMENTATION_STRATEGY_TO_STR.at(LengthGroupSegmentationStrategyType::SINGLE),
        ch_segmentation_strategy_str = CHANNEL_SEGMENTATION_STRATEGY_TO_STR.at(ChannelSegmentationStrategyType::SINGLE),
        segmentation_strategy_str = SEGMENTATION_STRATEGY_TO_STR.at(UNIFORM), score_based_weights_file = "",
        num_seg_props_file = "", split_strategy_str = ISAX_SPLIT_STRATEGY_TO_STR.at(ENTROPY_MAXIMIZING),
        breakpoint_strategy_str = ISAX_BREAKPOINT_STRATEGY_TO_STR.at(EQUIPROBABLE),
        index_format_str = ARCHIVE_TYPE_TO_STR.at(BINARY), search_type_str = SEARCH_TYPE_TO_STR.at(KNN),
        distance_measure_str = DISTANCE_TYPE_TO_STR.at(ED), inserter_type_str = ENTRY_INSERTER_TYPE_TO_STR.at(PARALLEL),
        entry_merger_type_str = ENTRY_MERGER_TYPE_TO_STR.at(DUMMY);
    vec<str> csv_paths;
    Real step_sd = R(1.0), noise = R(0.1), score_based_prop_exp = R(1.0), score_based_sample_frac = R(0.01),
         index_sample_frac = R(1.0);
    SaxNumBitsT first_layer_num_bits = 1, num_bits_limit = MAX_NUM_BITS_LIMIT, merger_num_bits = MAX_NUM_BITS_LIMIT;
    SaxSegIndT num_segments;
    uint num_series = 0, series_len, num_queries, l_min = 0, l_max = 0, pos_per_env = 0, l_per_group = 0,
         num_l_groups = 0, knn_k = 1, seed = 0, num_lags = 5, score_based_segment_len = 1;
    Real r_range_r = 1.0;
    size_t leaf_capacity = 0, max_leaves_to_visit = 0;
    vec<uint> exact_lengths = {};
    MtsNumChannelsT num_channels, used_channels = 0;
    vec<bool> channel_mask;
    bool zero_start = false, unnormalized = false, approximate = false, early_abandon = false, sort_query = false,
         no_use_pq = false, adapt_index = false, merge_in_leaves = false, prefer_first_in_em = false,
         separate_segment_stats = false;

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

    // Options for calculating dataset statistics
    d_stats_subcommand->add_option("-d,--dataset", dataset_path, "Dataset path relative to `DATA`")->required();
    d_stats_subcommand->add_option("-m,--series_len", series_len, "Length of series")->required()->check(positive_int);
    d_stats_subcommand->add_option("-c,--num_channels", num_channels, "Number of channels")
        ->required()
        ->check(positive_int);
    d_stats_subcommand
        ->add_option("--num_lags", num_lags, "Number of lags to calculate for autocorrelation and total variance")
        ->capture_default_str()
        ->check(positive_int);

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
    index_subcommand
        ->add_option("-G,--lg_segmentation_strategy", lg_segmentation_strategy_str,
                     "Length group segmentation strategy to use")
        ->capture_default_str()
        ->check(CLI::IsMember(ACCEPTED_LENGTH_GROUP_SEGMENTATION_STRATEGY_STRS));
    index_subcommand
        ->add_option("-C,--ch_segmentation_strategy", ch_segmentation_strategy_str,
                     "Channel segmentation strategy to use")
        ->capture_default_str();
    index_subcommand
        ->add_option("-S,--segmentation_strategy", segmentation_strategy_str, "Segmentation strategy to use")
        ->capture_default_str()
        ->check(CLI::IsMember(ACCEPTED_SEGMENTATION_STRATEGY_STRS));
    index_subcommand->add_option("-w,--score_based_weights_file", score_based_weights_file,
                                 "Path to the file containing the weights for the ScoreBasedChSegmentationStrategy");
    index_subcommand
        ->add_option("-e,--score_based_prop_exp", score_based_prop_exp,
                     "Exponent to use for the ScoreToProportionalNumSegments in ScoreBasedChSegmentationStrategy")
        ->capture_default_str();
    index_subcommand
        ->add_option(
            "--score_based_sample_frac", score_based_sample_frac,
            "Fraction of the dataset to use for estimating envelope statistics in ScoreBasedChSegmentationStrategy")
        ->capture_default_str()
        ->check(fraction);
    index_subcommand
        ->add_option(
            "--score_based_segment_len", score_based_segment_len,
            "Length of the segments to use for estimating envelope statistics in ScoreBasedChSegmentationStrategy")
        ->capture_default_str()
        ->check(positive_int);
    index_subcommand
        ->add_option(
            "--num_seg_props_file", num_seg_props_file,
            "Path to the file containing the proportions of segments per channel, use in MultiChSegmentationStrategy")
        ->capture_default_str();
    index_subcommand->add_option("-B,--breakpoint_strategy", breakpoint_strategy_str, "Breakpoint strategy")
        ->capture_default_str()
        ->check(CLI::IsMember(ACCEPTED_ISAX_BREAKPOINT_STRATEGY_STRS));
    index_subcommand->add_option("--split_strategy", split_strategy_str, "Split strategy")
        ->capture_default_str()
        ->check(CLI::IsMember(ACCEPTED_ISAX_SPLIT_STRATEGY_STRS));
    index_subcommand->add_flag("--merge_in_leaves", merge_in_leaves, "Merge entries in the leaves of the iSAX trie.");
    index_subcommand->add_flag("--prefer_first_in_em", prefer_first_in_em,
                               "Prefer the first segment over the one with the minimum number of bits, in case of ties "
                               "in the split when using EntropyMaximizing strategy");
    index_subcommand->add_option("--breakpoints", breakpoints_path, "Path to breakpoints file")->capture_default_str();
    index_subcommand->add_flag("--adapt", adapt_index, "Adapt the index properties to the dataset");
    index_subcommand->add_option("-l,--l_min", l_min, "Minimum length of subsequences")
        ->required()
        ->check(positive_int);
    index_subcommand->add_option("-L,--l_max", l_max, "Maximum length of subsequences")
        ->required()
        ->check(positive_int);
    index_subcommand->add_option("-s,--num_segments", num_segments, "Number of segments")
        ->required()
        ->check(positive_int);
    index_subcommand->add_option("-p,--pos_per_env", pos_per_env, "Positions per envelope")
        ->capture_default_str()
        ->check(positive_int);
    index_subcommand
        ->add_option("-g,--l_per_group", l_per_group,
                     "Lengths per group, 0 by default, indicating no length-based grouping")
        ->capture_default_str()
        ->check(positive_int);
    index_subcommand
        ->add_option("--bucket_size,--leaf_capacity", leaf_capacity,
                     "Leaf capacity or bucket size in case of tree envelope indexes")
        ->capture_default_str()
        ->check(positive_int);
    index_subcommand->add_flag("--raw", unnormalized, "Do not normalize");
    index_subcommand
        ->add_option("-b,--first_layer_bits", first_layer_num_bits,
                     "Number of bits for first layer in the case of iSAX, number of bits in the case of flat SAX "
                     "envelope and number of bits for the invSAX representation in case of tree envelope.")
        ->check(positive_int)
        ->capture_default_str();
    index_subcommand->add_option("--num_bits_limit", num_bits_limit, "Maximum number of bits per segment")
        ->check(positive_int)
        ->capture_default_str();
    index_subcommand->add_option("-I,--inserter_type", inserter_type_str, "Entry inserter type")
        ->capture_default_str()
        ->check(CLI::IsMember(ACCEPTED_ENTRY_INSERTER_TYPE_STRS));
    index_subcommand->add_option("-M,--merger", entry_merger_type_str, "Entry merger type")
        ->capture_default_str()
        ->check(CLI::IsMember(ACCEPTED_ENTRY_MERGER_TYPE_STRS));
    index_subcommand
        ->add_option(
            "--merger_num_bits", merger_num_bits,
            "Number of bits to use for SAX-based entry mergers. If not provided, takes the value of `num_bits_limit`.")
        ->capture_default_str()
        ->check(positive_int);
    index_subcommand
        ->add_option("--index_sample_frac", index_sample_frac,
                     "Fraction of the dataset to index, intended for testing, "
                     "defaults to 1.0, meaning that the whole dataset is indexed")
        ->capture_default_str()
        ->check(fraction);
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
    i_stats_subcommand
        ->add_option("-g,--num_l_groups", num_l_groups,
                     "Number of length groups, defaults to 0, indicating no "
                     "length-based grouping")
        ->capture_default_str()
        ->check(positive_int);
    i_stats_subcommand->add_flag("--separate_segment_stats", separate_segment_stats,
                                 "Calculate segment statistics for each segment separately");
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
    search_subcommand
        ->add_option("-g,--l_per_group", l_per_group,
                     "Lengths per group, 0 by default, indicating no length-based grouping")
        ->capture_default_str()
        ->check(positive_int);
    search_subcommand->add_option("-l,--l_min", l_min, "Minimum length of subsequences")
        ->capture_default_str()
        ->check(positive_int);
    search_subcommand->add_option("-L,--l_max", l_max, "Maximum length of subsequences")
        ->capture_default_str()
        ->check(positive_int);
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
        ->capture_default_str();
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
    bool use_length_groups = l_per_group > 0;

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
        if (method_type == TREE_ENVELOPE) {
            if (leaf_capacity < 2) {
                std::cerr << "--leaf_capacity (bucket size) must be greater than 1\n";
                return 1;
            }
        }
        if (method_type == ISAX || method_type == ISAX_ENVELOPE || method_type == SAX_ENVELOPE) {
            // When using fixed breakpoints strategy, the breakpoints file must exist and must contain sufficient
            // breakpoints
            SaxBreakpointStrategyType breakpoint_strategy_type =
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
        if (use_length_groups) {
            if (l_min == 0 || l_max == 0 || l_min > l_max) {
                std::cerr << "When using length-based grouping, --l_min and --l_max must be provided\n";
                return 1;
            }
        }
    }

    // Initialize run settings
    try {
        RunSettings::initialize(command_type, {num_channels, series_len, num_series, dataset_path},
                                {use_length_groups, l_min, l_max, l_per_group, num_l_groups}, pos_per_env, index_path,
                                ffts_path, query_path, method_type, logs_path);
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
        case CALC_D_STATS: {
            return calculate_dataset_stats(num_lags);
        }
        case CREATE_QS: {
            return create_queries({noise, num_queries, exact_lengths, l_min, l_max, used_channels, channel_mask, seed});
        }
        case CALC_Q_STATS: {
            return calculate_query_stats(!unnormalized);
        }
        case INDEX: {
            IIndexParams *index_params;

            auto lg_segmentation_strategy_type =
                STR_TO_LENGTH_GROUP_SEGMENTATION_STRATEGY.at(lg_segmentation_strategy_str);
            auto ch_segmentation_strategy_type = STR_TO_CHANNEL_SEGMENTATION_STRATEGY.at(ch_segmentation_strategy_str);
            auto segmentation_strategy_type = STR_TO_SEGMENTATION_STRATEGY.at(segmentation_strategy_str);
            auto breakpoint_strategy_type = STR_TO_ISAX_BREAKPOINT_STRATEGY.at(breakpoint_strategy_str);
            auto split_strategy_type = STR_TO_ISAX_SPLIT_STRATEGY.at(split_strategy_str);
            auto env_entry_merger_type = STR_TO_ENTRY_MERGER_TYPE.at(entry_merger_type_str);

            uptr<SaxParams> merger_sax_params = nullptr;
            if (arr_contains(MERGERS_W_SAX, env_entry_merger_type)) {
                if (arr_contains(METHODS_W_ISAX, method_type) && merger_num_bits < num_bits_limit) {
                    std::cout << "Warning: The number of bits for the SAX-based merger is less than the bit limit for "
                                 "the iSAX trie. The merger will use "
                              << U(num_bits_limit) << " bits (instead of " << U(merger_num_bits) << ").\n";
                    merger_num_bits = num_bits_limit;
                }
                merger_sax_params =
                    std::make_unique<SaxParams>(merger_num_bits, breakpoint_strategy_type, breakpoints_path);
            }

            uptr<ScoreBasedChSSParams> score_based_ch_ss_params = nullptr;
            if (ch_segmentation_strategy_type == ChannelSegmentationStrategyType::SCORE_BASED) {
                score_based_ch_ss_params = std::make_unique<ScoreBasedChSSParams>(
                    score_based_segment_len, score_based_sample_frac, score_based_prop_exp, score_based_weights_file);
            }
            SegmentationParams segmentation_params{
                .m_num_segments = num_segments,
                .m_lg_strategy_type = lg_segmentation_strategy_type,
                .m_ch_strategy_type = ch_segmentation_strategy_type,
                .m_strategy_type = segmentation_strategy_type,
                .m_ch_score_based_params = score_based_ch_ss_params.get(),
                .m_ch_num_seg_props_file = num_seg_props_file,
            };
            SaxParams sax_params{
                .m_num_bits = first_layer_num_bits,
                .m_breakpoint_strategy_type = breakpoint_strategy_type,
                .m_breakpoints_file = breakpoints_path,
            };
            MergerParams merger_params{
                .m_entry_merger_type = env_entry_merger_type,
                .m_merger_sax_params = merger_sax_params.get(),
            };
            iSaxTrieParams isax_trie_params{
                .m_merge_in_leaves = merge_in_leaves,
                .m_min_num_bits_on_tie = !prefer_first_in_em,
                .m_num_bits_limit = num_bits_limit,
                .m_split_strategy_type = split_strategy_type,
                .m_leaf_capacity = leaf_capacity,
            };

            switch (method_type) {
                case ISAX_ENVELOPE:
                case ISAX_ENV_W_ENV:
                case ISAX_ENV_W_SAX_ENV:
                    index_params = new iSaxEnvelopeIndexParams(segmentation_params, merger_params, pos_per_env,
                                                               sax_params, isax_trie_params);
                    break;
                case ISAX:
                    index_params =
                        new iSaxIndexParams(segmentation_params, merger_params, sax_params, isax_trie_params);
                    break;
                case ENVELOPE:
                    index_params = new EnvelopeIndexParams(segmentation_params, merger_params, pos_per_env);
                    break;
                case SAX_ENVELOPE:
                    index_params =
                        new SaxEnvelopeIndexParams(segmentation_params, merger_params, pos_per_env, sax_params);
                    break;
                case TREE_ENVELOPE:
                    index_params = new TreeEnvelopeIndexParams(segmentation_params, merger_params, pos_per_env,
                                                               sax_params, leaf_capacity);
                    break;
                case SEQUENTIAL_SCAN:
                    std::cerr << "Sequential scan does not require indexation\n";
                    return 1;
            }
            IndexOptions index_options{
                .m_normalized = !unnormalized,
                .m_adapt = adapt_index,
                .m_use_length_groups = use_length_groups,
                .m_num_channels = num_channels,
                .m_index_method = method_type,
                .m_index_format = STR_TO_ARCHIVE_TYPE.at(index_format_str),
                .m_inserter_type = STR_TO_ENTRY_INSERTER_TYPE.at(inserter_type_str),
                .m_l_min = l_min,
                .m_l_max = l_max,
                .m_series_len = series_len,
                .m_l_per_group = l_per_group,
                .m_index_params = std::unique_ptr<IIndexParams>(index_params),
            };
            return create_index(index_options, index_sample_frac);
        }
        case CALC_I_STATS: {
            return calculate_index_stats(method_type, num_l_groups, STR_TO_ARCHIVE_TYPE.at(index_format_str),
                                         separate_segment_stats);
        }
        case CALC_FFTS: {
            return calculate_ffts(!unnormalized);
        }
        case SEARCH: {
            SearchType search_type = STR_TO_SEARCH_TYPE.at(search_type_str);
            DistanceType distance_type = STR_TO_DISTANCE_TYPE.at(distance_measure_str);

            SearchOptions search_options = {
                .m_exact = !approximate,
                .m_normalized = !unnormalized,
                .m_use_early_abandoning = early_abandon,
                .m_sort_queries = sort_query,
                .m_use_priority_queue = !no_use_pq,
                .m_use_length_groups = use_length_groups,
                .m_search_method_type = STR_TO_SEARCH_METHOD_TYPE.at(search_method_type_str),
                .m_index_format = STR_TO_ARCHIVE_TYPE.at(index_format_str),
                .m_search_type = search_type,
                .m_distance_type = distance_type,
                .m_l_min = l_min,
                .m_l_max = l_max,
                .m_l_per_group = l_per_group,
                .m_knn_k = knn_k,
                .m_r_range_r = r_range_r,
                .m_max_leaves_to_visit = max_leaves_to_visit,
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

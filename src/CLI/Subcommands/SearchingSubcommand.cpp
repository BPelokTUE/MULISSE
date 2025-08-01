#include "CLI/Subcommands/SearchingSubcommand.hpp"

#include "CLI/CommonOptions.hpp"
#include "CLI/Validators.hpp"
#include "Enums/CommandType.hpp"
#include "Modules/Searching.hpp"
#include "Search/DistanceMeasure/DistanceMeasure.hpp"
#include "Search/DistanceMeasure/EuclideanDistance.hpp"
#include "Search/DistanceMeasure/Mass.hpp"
#include "Search/Results/ResultSet.hpp"
#include "Search/SearchOptions.hpp"

SearchingSubcommand::SearchingSubcommand(CLI::App &app) {
    auto search_subcommand = app.add_subcommand(CMD_TYPE_TO_STR.at(SEARCH), "Search using MULISSE");
    auto &validators = validators::get_instance();
    auto &positive_int = validators.get_positive_int_validator();
    auto &positive_real = validators.get_positive_real_validator();

    search_subcommand->add_option("-i,--index_meta", m_index_meta_path, "Path to meta file of the index to use, if any")
        ->capture_default_str();
    search_subcommand->add_option("-d,--dataset", m_dataset_meta_path, "Path to the meta file of the dataset to search")
        ->required();
    search_subcommand->add_option("-q,--query", m_query_meta_path, "Path to the meta file of the queryset to answer")
        ->required();
    search_subcommand
        ->add_option("-F,--ffts", m_ffts_meta_path,
                     "Path to load FFTs from relative to `DATA`; if not provided, FFTs will not be loaded")
        ->capture_default_str();
    search_subcommand->add_option("-D,--distance", m_distance_measure_str, "Distance measure")
        ->capture_default_str()
        ->check(CLI::IsMember(ACCEPTED_DISTANCE_TYPE_STRS));
    search_subcommand->add_flag("--early_abandon", m_early_abandon, "Use early abandoning");
    search_subcommand->add_flag(
        "--sort_query", m_sort_query,
        "Sort data points of queries based on their absolute values. Only supported for Euclidean distance "
        "with early abandoning.");
    search_subcommand->add_flag("--examine_whole", m_examine_whole,
                                "Examine the whole series when a subsequence examination is performed");
    search_subcommand->add_flag("--no_pq,--no_priority_queue", m_no_use_pq,
                                "Do not use a priority queue for flat envelope index search");
    search_subcommand->add_flag("--approx", m_approximate, "Approximate search");
    search_subcommand
        ->add_option(
            "-M,--max_leaves_to_visit", m_max_leaves_to_visit,
            "Maximum number of leaves to visit if approximate search is used. Defaults to 0, indicating no max.")
        ->capture_default_str();
    //      Search type-specific options
    search_subcommand->add_option("-T,--search_type", m_search_type_str, "Search type")
        ->capture_default_str()
        ->check(CLI::IsMember(ACCEPTED_SEARCH_TYPE_STRS));
    search_subcommand->add_option("-k,--k", m_knn_k, "Number of nearest neighbors for kNN")
        ->capture_default_str()
        ->check(positive_int);
    search_subcommand->add_option("-r,--range", m_r_range_r, "Range for range search")
        ->capture_default_str()
        ->check(positive_real);
}

void SearchingSubcommand::execute(const CommonOptions &common_opts) {
    SearchType search_type = STR_TO_SEARCH_TYPE.at(m_search_type_str);
    DistanceType distance_type = STR_TO_DISTANCE_TYPE.at(m_distance_measure_str);

    SearchOptions search_options = {
        .m_exact = !m_approximate,
        .m_normalized = !common_opts.m_raw,
        .m_use_early_abandoning = m_early_abandon,
        .m_sort_queries = m_sort_query,
        .m_examine_whole = m_examine_whole,
        .m_use_priority_queue = !m_no_use_pq,
        // .m_search_method_type = STR_TO_SEARCH_METHOD_TYPE.at(m_search_method_type_str),
        // .m_index_format = STR_TO_ARCHIVE_TYPE.at(m_index_format_str),
        .m_search_type = search_type,
        .m_distance_type = distance_type,
        .m_knn_k = m_knn_k,
        .m_r_range_r = m_r_range_r,
        .m_max_leaves_to_visit = m_max_leaves_to_visit,
        // .m_sax_params =
        //     SaxParams{
        //         .m_num_bits = m_breakpoint_num_bits,
        //         .m_breakpoint_strategy_type = STR_TO_ISAX_BREAKPOINT_STRATEGY.at(m_breakpoint_strategy_str),
        //         .m_breakpoints_file = m_breakpoints_path,
        //     },
    };

    // Precomputed FFTs always require examining the whole series
    // m_examine_whole |= !ffts_path.empty();

    switch (distance_type) {
        case ED:
            if (search_type == KNN) {
                ResultSet<KNN> knn_result_set(m_knn_k);
                if (m_sort_query) {
                    DistanceMeasure<KNN, ED, true> distance_measure(!common_opts.m_raw, m_early_abandon);
                    if (m_examine_whole)
                        search<KNN, ED, true, true>(search_options, knn_result_set, distance_measure);
                    else
                        search<KNN, ED, false, true>(search_options, knn_result_set, distance_measure);
                } else {
                    DistanceMeasure<KNN, ED> distance_measure(!common_opts.m_raw, m_early_abandon);
                    if (m_examine_whole)
                        search<KNN, ED, true>(search_options, knn_result_set, distance_measure);
                    else
                        search<KNN, ED>(search_options, knn_result_set, distance_measure);
                }
            } else {  // search_type == R_RANGE
                ResultSet<R_RANGE> result_set(m_r_range_r);
                DistanceMeasure<R_RANGE, ED> distance_measure(!common_opts.m_raw, m_early_abandon);
                if (m_examine_whole)
                    search<R_RANGE, ED, true>(search_options, result_set, distance_measure);
                else
                    search<R_RANGE, ED>(search_options, result_set, distance_measure);
            }
        case MASS:
            if (STR_TO_SEARCH_TYPE.at(m_search_type_str) == KNN) {
                ResultSet<KNN> knn_result_set(m_knn_k);
                DistanceMeasure<KNN, MASS> distance_measure(!common_opts.m_raw);
                if (m_examine_whole)
                    search<KNN, MASS, true>(search_options, knn_result_set, distance_measure);
                else
                    search<KNN, MASS>(search_options, knn_result_set, distance_measure);
            } else {  // search_type == R_RANGE
                ResultSet<R_RANGE> result_set(m_r_range_r);
                DistanceMeasure<R_RANGE, MASS> distance_measure(!common_opts.m_raw);
                if (m_examine_whole)
                    search<R_RANGE, MASS, true>(search_options, result_set, distance_measure);
                else
                    search<R_RANGE, MASS>(search_options, result_set, distance_measure);
            }
        default:
            throw std::runtime_error("Distance measure \"" + m_distance_measure_str + "\" is not implemented\n");
    }
}

#include "CLI/Subcommands/CreateQueriesSubcommand.hpp"

#include "CLI/CommonOptions.hpp"
#include "CLI/Validators.hpp"
#include "Enums/CommandType.hpp"
#include "Modules/QueryGen.hpp"
#include "Util/HelperFuncs/Errors.hpp"

CreateQueriesSubcommand::CreateQueriesSubcommand(CLI::App &app) {
    auto qs_subcommand = app.add_subcommand(CMD_TYPE_TO_STR.at(CREATE_QS), "Create queries from dataset");

    qs_subcommand->add_option("-d,--dataset_meta", m_dataset_meta_path, "Path to the meta file of the dataset")
        ->required()
        ->check(validators::file_is_readable);
    qs_subcommand->add_option("-q,--query", m_queryset_path, "Output query path relative to `DATA`")
        ->required()
        ->check(validators::file_is_writable);
    qs_subcommand->add_option("--noise", m_queryset_opts.m_noise, "Query noise")->capture_default_str();
    qs_subcommand->add_option("-Q,--num_queries", m_queryset_opts.m_num_queries, "Number of queries")
        ->required()
        ->check(validators::positive_int);
    qs_subcommand
        ->add_option("-e,--exact_lengths", m_queryset_opts.m_exact_lengths,
                     "List of query lengths to generate. Is overriden by `--l_min` and `--l_max`.")
        ->capture_default_str()
        ->check(validators::positive_int);
    qs_subcommand
        ->add_option(
            "-l,--l_min", m_queryset_opts.m_l_min,
            "Minimum length of queries to generate. If passed `--l_max` is also required. Overrides `--exact_lengths`.")
        ->capture_default_str();
    qs_subcommand
        ->add_option(
            "-L,--l_max", m_queryset_opts.m_l_max,
            "Maximum length of queries to generate. If passed `--l_min` is also required. Overrides `--exact_lengths`.")
        ->capture_default_str();
    qs_subcommand
        ->add_option("-u,--used_channels", m_queryset_opts.m_used_channels,
                     "Number of channels to use for queries. 0 by default, meaning that the number of used "
                     "channels is selected randomly for each query.")
        ->capture_default_str();
    qs_subcommand
        ->add_option("-M,--channel_mask", m_queryset_opts.m_channel_mask,
                     "Mask for which channels to use in the queries. Overrides "
                     "used_channels if provided.")
        ->capture_default_str();
}

void CreateQueriesSubcommand::set_up_execution(const CommonOptions *common_opts) {
    // Set up run setting properties
}

void CreateQueriesSubcommand::validate_arguments() {
    if (m_queryset_opts.m_l_min > m_queryset_opts.m_l_max) {
        throw get_l_min_gt_l_max_error();
    }
    // if (size_t mask_size = m_queryset_opts.m_channel_mask.size(); mask_size > 0 && mask_size != m_num_channels) {
    //     throw std::runtime_error("Non-empty channel mask has different number of channels (" + to_string(mask_size) +
    //                              ") than dataset (" + to_string(m_num_channels) + ")");
    // }
}

void CreateQueriesSubcommand::execute() {
    m_queryset_opts.m_seed = m_common_opts->m_seed;
    create_queries(m_queryset_opts);
}

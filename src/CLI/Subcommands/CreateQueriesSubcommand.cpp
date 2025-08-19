#include "CLI/Subcommands/CreateQueriesSubcommand.hpp"

#include "CLI/Validators.hpp"
#include "Enums/CommandType.hpp"
#include "Modules/QueryGen.hpp"
#include "Util/Artefacts/MtsQuerySet.hpp"
#include "Util/HelperFuncs/Errors.hpp"
#include "Util/HelperFuncs/Path.hpp"
#include "Util/Logging/QuerySetLogger.hpp"
#include "Util/Types/RunContext.hpp"

CreateQueriesSubcommand::CreateQueriesSubcommand(CLI::App &app) {
    auto qs_subcommand = app.add_subcommand(CMD_TYPE_TO_STR.at(CREATE_QS), "Create queries from dataset");

    qs_subcommand->add_option("-d,--dataset_meta", m_dataset_meta_path, "Path to the meta file of the dataset")
        ->required()
        ->check(validators::file_is_readable);
    qs_subcommand->add_option("-q,--query", m_query_set_path, "Output query path relative to `DATA`")
        ->required()
        ->check(validators::file_is_writable);
    qs_subcommand->add_option("--noise", m_query_gen_opts.m_noise, "Query noise")->capture_default_str();
    qs_subcommand->add_option("-Q,--num_queries", m_query_set_props.m_num_queries, "Number of queries")
        ->required()
        ->check(validators::positive_int);
    qs_subcommand
        ->add_option("-e,--exact_lengths", m_query_gen_opts.m_exact_lengths,
                     "List of query lengths to generate. Is overriden by `--l_min` and `--l_max`.")
        ->capture_default_str()
        ->check(validators::positive_int);
    qs_subcommand
        ->add_option(
            "-l,--l_min", m_query_set_props.m_length_range.m_l_min,
            "Minimum length of queries to generate. If passed `--l_max` is also required. Overrides `--exact_lengths`.")
        ->capture_default_str();
    qs_subcommand
        ->add_option(
            "-L,--l_max", m_query_set_props.m_length_range.m_l_max,
            "Maximum length of queries to generate. If passed `--l_min` is also required. Overrides `--exact_lengths`.")
        ->capture_default_str();
    qs_subcommand
        ->add_option("-u,--used_channels", m_query_gen_opts.m_used_channels,
                     "Number of channels to use for queries. 0 by default, meaning that the number of used "
                     "channels is selected randomly for each query.")
        ->capture_default_str();
    qs_subcommand
        ->add_option("-M,--channel_mask", m_query_gen_opts.m_channel_mask,
                     "Mask for which channels to use in the queries. Overrides "
                     "used_channels if provided.")
        ->capture_default_str();
}

void CreateQueriesSubcommand::execute(const RunContext &run_context) {
    // Set up dataset
    m_dataset.load_meta(std::filesystem::path(m_run_context->m_data_path) / m_dataset_meta_path);
    m_dataset.set_istream(std::make_unique<std::ifstream>(std::filesystem::path(m_run_context->m_data_path) /
                                                          m_dataset.get_properties().m_dataset_path));

    // Do extra argument validation

    //// Check number of channels
    MtsNumChannelsT num_channels = m_dataset.get_properties().m_num_channels;
    if (size_t mask_size = m_query_gen_opts.m_channel_mask.size(); mask_size > 0 && mask_size != num_channels) {
        throw std::runtime_error(std::format(
            "Non-empty channel mask has different number of channels ({}) from dataset ({})", mask_size, num_channels));
    }

    //// Check channel selection
    if (!m_query_gen_opts.m_channel_mask.empty() && m_query_gen_opts.m_channel_mask.size() != num_channels) {
        throw std::runtime_error(
            std::format("Channel mask must have the same number of elements as the number of channels in the "
                        "dataset ({}), but has {} instead.",
                        num_channels, m_query_gen_opts.m_channel_mask.size()));
    } else if (m_query_gen_opts.m_used_channels != 0 && m_query_gen_opts.m_used_channels > num_channels) {
        throw std::runtime_error(std::format(
            "Number of used channels ({}) must be less than or equal to the number of channels in the dataset ({}).",
            m_query_gen_opts.m_used_channels, num_channels));
    }

    //// Check length specification
    if (m_query_set_props.m_length_range.m_l_min <= 0 ||
        m_query_set_props.m_length_range.m_l_max < m_query_set_props.m_length_range.m_l_min) {
        if (m_query_gen_opts.m_exact_lengths.empty()) {
            throw std::runtime_error("Either a list of exact lengths or a minimum and maximum length must be provided");
        } else {
            m_query_set_props.m_length_range = {0, 0};
        }
    } else {
        m_query_gen_opts.m_exact_lengths.clear();
    }

    // Set up query set and generation properties
    m_query_gen_opts.m_seed = m_run_context->m_seed;
    auto ofs = std::make_unique<std::ofstream>(std::filesystem::path(m_run_context->m_data_path) / m_query_set_path);
    MtsQuerySet query_set(m_dataset, m_query_set_props, std::move(ofs));

    // Create logger
    QuerySetLogger logger(run_context.m_logs_path);

    // Generate queries
    create_queries(query_set, m_query_gen_opts, logger);

    // Save query set meta
    query_set.save_meta(query_set.get_meta_path());
}

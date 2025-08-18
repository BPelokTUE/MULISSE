#include "CLI/Subcommands/ParseCsvSubcommand.hpp"

#include "CLI/Validators.hpp"
#include "Enums/CommandType.hpp"
#include "Modules/CsvParsing.hpp"
#include "Util/Artefacts/MtsDataset.hpp"
#include "Util/HelperFuncs/Errors.hpp"
#include "Util/Logging/DatasetLogger.hpp"
#include "Util/Types/LengthRange.hpp"
#include "Util/Types/RunContext.hpp"

ParseCsvSubcommand::ParseCsvSubcommand(CLI::App &app) {
    auto csv_subcommand = app.add_subcommand(CMD_TYPE_TO_STR.at(PARSE_CSV), "Create dataset from CSV");

    csv_subcommand
        ->add_option("-i,--input", m_csv_gen_opts.m_source_csvs, "Input CSV file paths, in the order of channels")
        ->required()
        ->check(validators::file_is_readable);
    csv_subcommand->add_option("-d,--dataset", m_dataset_props.m_dataset_path, "Output dataset path relative to `DATA`")
        ->required();
    csv_subcommand->add_option("-n,--num_series", m_dataset_props.m_num_series, "Max number of series")
        ->required()
        ->check(validators::positive_int);
    csv_subcommand->add_option(
        "-l,--l_min", m_csv_gen_opts.m_l_range.m_l_min,
        "Minimum length of subsequences that will be queried for. Used for discarding series with "
        "stagnant subsequences that would make normalization unstable");
    csv_subcommand->add_option("-L,--l_max", m_csv_gen_opts.m_l_range.m_l_max,
                               "Maximum length of subsequences that will be queried for. Used for discarding stagnant "
                               "subsequences that would make normalization unstable");
    csv_subcommand
        ->add_option("-s,--min_subs_sd", m_csv_gen_opts.m_min_subs_sd,
                     "Minimum standard deviation required for each subsequence")
        ->capture_default_str()
        ->check(validators::non_negative_real);
    csv_subcommand->add_option("-m,--series_len", m_dataset_props.m_series_len, "Length of series")
        ->required()
        ->check(validators::positive_int);
}

void ParseCsvSubcommand::execute(const RunContext &run_context) {
    // Validate
    m_csv_gen_opts.m_l_range.validate();

    // Set up dataset generation
    m_dataset_props.m_num_channels = static_cast<MtsNumChannelsT>(m_csv_gen_opts.m_source_csvs.size());
    MtsDataset dataset(m_dataset_props);
    dataset.set_up_generation(run_context.m_data_path);

    m_csv_gen_opts.m_seed = run_context.m_seed;

    DatasetLogger logger(run_context.m_logs_path);

    // Generate time series
    create_dataset_from_csv(dataset, m_csv_gen_opts, logger);

    // Save dataset meta
    dataset.save_meta(dataset.get_meta_path());
}

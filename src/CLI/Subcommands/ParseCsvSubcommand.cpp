#include "CLI/Subcommands/ParseCsvSubcommand.hpp"

#include "CLI/CommonOptions.hpp"
#include "CLI/Validators.hpp"
#include "Enums/CommandType.hpp"
#include "Modules/CsvParsing.hpp"
#include "Util/Artefacts/MtsDataset.hpp"
#include "Util/HelperFuncs/Errors.hpp"
#include "Util/Types/LengthRange.hpp"

ParseCsvSubcommand::ParseCsvSubcommand(CLI::App &app) {
    auto csv_subcommand = app.add_subcommand(CMD_TYPE_TO_STR.at(PARSE_CSV), "Create dataset from CSV");

    csv_subcommand->add_option("-i,--input", m_csv_paths, "Input CSV file paths, in the order of channels")->required();
    csv_subcommand->add_option("-d,--dataset", m_settings.m_dataset_path, "Output dataset path relative to `DATA`")
        ->required();
    csv_subcommand->add_option("-n,--num_series", m_settings.m_num_series, "Max number of series")
        ->required()
        ->check(validators::positive_int);
    csv_subcommand->add_option(
        "-l,--l_min", m_l_min,
        "Minimum length of subsequences that will be queried for. Used for discarding series with "
        "stagnant subsequences that would make normalization unstable");
    csv_subcommand->add_option("-L,--l_max", m_l_max,
                               "Maximum length of subsequences that will be queried for. Used for discarding stagnant "
                               "subsequences that would make normalization unstable");
    csv_subcommand
        ->add_option("-s,--min_subs_sd", m_min_subs_sd, "Minimum standard deviation required for each subsequence")
        ->capture_default_str()
        ->check(validators::non_negative_real);
    csv_subcommand->add_option("-m,--series_len", m_settings.m_series_len, "Length of series")
        ->required()
        ->check(validators::positive_int);
}

void ParseCsvSubcommand::set_up_execution(const CommonOptions *common_opts) {
    m_common_opts = common_opts;
    m_settings.m_dataset_path = std::filesystem::path(m_common_opts->m_data_path) / m_settings.m_dataset_path;
    m_settings.m_num_channels = static_cast<MtsNumChannelsT>(m_csv_paths.size());
}

void ParseCsvSubcommand::validate_arguments() {
    if (m_l_min > m_l_max) throw get_l_min_gt_l_max_error({m_l_min, m_l_max});
}

void ParseCsvSubcommand::execute() {
    MtsDataset dataset(m_settings);
    create_dataset_from_csv(dataset, m_csv_paths, m_l_min, m_l_max, ',', m_min_subs_sd, m_common_opts->m_seed);
    dataset.save_meta(dataset.get_meta_path());
}

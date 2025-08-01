#include "CLI/Subcommands/DatasetStatsSubcommand.hpp"

#include "CLI/CommonOptions.hpp"
#include "CLI/Validators.hpp"
#include "Enums/CommandType.hpp"
#include "Modules/CalcDatasetStats.hpp"

DatasetStatsSubcommand::DatasetStatsSubcommand(CLI::App &app) {
    auto d_stats_subcommand = app.add_subcommand(CMD_TYPE_TO_STR.at(CALC_D_STATS), "Calculate dataset statistics");

    d_stats_subcommand->add_option("-d,--dataset_meta", m_dataset_meta_path, "Path of the dataset meta file")
        ->required()
        ->check(validators::file_is_readable);
    d_stats_subcommand
        ->add_option("--num_lags", m_num_lags, "Number of lags to calculate for autocorrelation and total variance")
        ->capture_default_str()
        ->check(validators::positive_int);
}

void DatasetStatsSubcommand::set_up_execution(const CommonOptions *common_opts) {}

void DatasetStatsSubcommand::execute() {
    // Set up run setting properties
    calculate_dataset_stats(m_num_lags);
}

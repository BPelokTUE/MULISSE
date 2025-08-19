#include "CLI/Subcommands/DatasetStatsSubcommand.hpp"

#include "CLI/Validators.hpp"
#include "Enums/CommandType.hpp"
#include "Modules/CalcDatasetStats.hpp"
#include "Util/Logging/DatasetStatsLogger.hpp"
#include "Util/Types/RunContext.hpp"

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

void DatasetStatsSubcommand::execute(const RunContext &run_context) {
    // Set up dataset statistics calculation
    m_mts_dataset.load_meta(std::filesystem::path(run_context.m_data_path) / m_dataset_meta_path);
    m_mts_dataset.set_istream(std::make_unique<std::ifstream>(std::filesystem::path(run_context.m_data_path) /
                                                              m_mts_dataset.get_properties().m_dataset_path));

    DatasetStatsLogger logger(run_context.m_logs_path);

    // Calculate dataset statistics
    calculate_dataset_stats(m_mts_dataset, m_num_lags, logger);
}

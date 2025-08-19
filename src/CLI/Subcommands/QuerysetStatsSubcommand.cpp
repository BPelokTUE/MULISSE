#include "CLI/Subcommands/QuerySetStatsSubcommand.hpp"

#include "Enums/CommandType.hpp"
#include "Modules/CalcQueryStats.hpp"
#include "Util/Artefacts/MtsDataset.hpp"
#include "Util/Artefacts/MtsQuerySet.hpp"
#include "Util/Logging/QueryStatsLogger.hpp"
#include "Util/Types/RunContext.hpp"

namespace fs = std::filesystem;

QuerySetStatsSubcommand::QuerySetStatsSubcommand(CLI::App &app) {
    auto q_stats_subcommand = app.add_subcommand(CMD_TYPE_TO_STR.at(CALC_Q_STATS), "Calculate query statistics");

    q_stats_subcommand->add_option("-d,--dataset_meta", m_dataset_meta_path, "Dataset path relative to `DATA`")
        ->required();
    q_stats_subcommand->add_option("-q,--query_meta", m_query_meta_path, "Query path relative to `DATA`")->required();
}

void QuerySetStatsSubcommand::execute(const RunContext &run_context) {
    // Set up statistics calculation
    MtsDataset dataset;
    dataset.load_meta(fs::path(run_context.m_data_path) / m_dataset_meta_path);
    dataset.set_istream(
        std::make_unique<std::ifstream>(fs::path(run_context.m_data_path) / dataset.get_properties().m_dataset_path));

    MtsQuerySet query_set;
    query_set.load_meta(fs::path(run_context.m_data_path) / m_query_meta_path);
    query_set.set_istream(std::make_unique<std::ifstream>(fs::path(run_context.m_data_path) /
                                                          query_set.get_properties().m_query_set_path));

    QueryStatsLogger logger(run_context.m_logs_path);

    // Calculate query statistics
    calculate_query_stats(query_set, run_context.m_normalized, logger);
}

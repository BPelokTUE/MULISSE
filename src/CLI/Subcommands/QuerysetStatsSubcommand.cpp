#include "CLI/Subcommands/QuerySetStatsSubcommand.hpp"

#include "CLI/CommonOptions.hpp"
#include "Enums/CommandType.hpp"
#include "Modules/CalcQueryStats.hpp"

QuerySetStatsSubcommand::QuerySetStatsSubcommand(CLI::App &app) {
    auto q_stats_subcommand = app.add_subcommand(CMD_TYPE_TO_STR.at(CALC_Q_STATS), "Calculate query statistics");

    q_stats_subcommand->add_option("-d,--dataset_meta", m_dataset_meta_path, "Dataset path relative to `DATA`")
        ->required();
    q_stats_subcommand->add_option("-q,--query_meta", m_query_meta_path, "Query path relative to `DATA`")->required();
}

void QuerySetStatsSubcommand::execute() {
    // Set up run setting properties
    calculate_query_stats(!m_common_opts->m_raw);
}

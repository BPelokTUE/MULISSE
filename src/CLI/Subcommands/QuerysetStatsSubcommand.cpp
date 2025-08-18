#include "CLI/Subcommands/QuerySetStatsSubcommand.hpp"

#include "Enums/CommandType.hpp"
#include "Modules/CalcQueryStats.hpp"
#include "Util/Artefacts/MtsDataset.hpp"
#include "Util/Artefacts/MtsQuerySet.hpp"
#include "Util/Types/RunContext.hpp"

QuerySetStatsSubcommand::QuerySetStatsSubcommand(CLI::App &app) {
    auto q_stats_subcommand = app.add_subcommand(CMD_TYPE_TO_STR.at(CALC_Q_STATS), "Calculate query statistics");

    q_stats_subcommand->add_option("-d,--dataset_meta", m_dataset_meta_path, "Dataset path relative to `DATA`")
        ->required();
    q_stats_subcommand->add_option("-q,--query_meta", m_query_meta_path, "Query path relative to `DATA`")->required();
}

void QuerySetStatsSubcommand::execute() {
    MtsDataset dataset;
    MtsQuerySet query_set;
    dataset.load_meta(m_dataset_meta_path);
    query_set.load_meta(m_query_meta_path);
    calculate_query_stats(dataset, query_set, *m_run_context);
}

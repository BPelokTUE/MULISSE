#include <fstream>
#include <iostream>

#include "CLI/Subcommands/CalculateFftsSubcommand.hpp"
#include "CLI/Subcommands/CreateQueriesSubcommand.hpp"
#include "CLI/Subcommands/DatasetStatsSubcommand.hpp"
#include "CLI/Subcommands/IndexStatsSubcommand.hpp"
#include "CLI/Subcommands/IndexingSubcommand.hpp"
#include "CLI/Subcommands/ParseCsvSubcommand.hpp"
#include "CLI/Subcommands/QuerySetStatsSubcommand.hpp"
#include "CLI/Subcommands/RandomWalkSubcommand.hpp"
#include "CLI/Subcommands/SearchingSubcommand.hpp"
#include "CLI/Validators.hpp"
#include "Enums/CommandType.hpp"
#include "Util/Types/RunContext.hpp"

int main(int argc, char **argv) {
    CLI::App app{"Run MULISSE"};

    app.require_subcommand(1);

    RunContext run_context;
    app.add_option("--seed", run_context.m_seed, "Random seed")->capture_default_str();
    app.add_option("--logs", run_context.m_logs_path, "Path to write logs to")->capture_default_str();
    app.add_option("--data", run_context.m_data_path, "Path to the data directory")->capture_default_str();
    app.add_flag("!--raw,--normalize", run_context.m_normalized, "Do not normalize");

    umap<CommandType, uptr<ISubcommand>> subcommands{
        {CREATE_DS, std::make_unique<RandomWalkSubcommand>(app)},
        {PARSE_CSV, std::make_unique<ParseCsvSubcommand>(app)},
        {CALC_D_STATS, std::make_unique<DatasetStatsSubcommand>(app)},
        {CREATE_QS, std::make_unique<CreateQueriesSubcommand>(app)},
        {CALC_Q_STATS, std::make_unique<QuerySetStatsSubcommand>(app)},
        {CALC_FFTS, std::make_unique<CalculateFftsSubcommand>(app)},
        {INDEX, std::make_unique<IndexingSubcommand>(app)},
        {CALC_I_STATS, std::make_unique<IndexStatsSubcommand>(app)},
        {SEARCH, std::make_unique<SearchingSubcommand>(app)},
    };

    // Parse arguments and initialize run settings
    CLI11_PARSE(app, argc, argv);
    CommandType selected_command_type = STR_TO_CMD_TYPE.at(app.get_subcommands().front()->get_name());

    // Delete all subcommands except the one to execute
    for (auto &[subcommand_type, subcommand_ptr] : subcommands) {
        if (subcommand_type != selected_command_type) subcommand_ptr.reset();
    }

    subcommands[selected_command_type]->execute(run_context);

    return 0;
}

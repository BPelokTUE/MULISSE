#include <fstream>
#include <iostream>

#include "CLI/CommonOptions.hpp"
#include "CLI/Subcommands/CalculateFftsSubcommand.hpp"
#include "CLI/Subcommands/CreateQueriesSubcommand.hpp"
#include "CLI/Subcommands/DatasetStatsSubcommand.hpp"
#include "CLI/Subcommands/IndexStatsSubcommand.hpp"
#include "CLI/Subcommands/IndexingSubcommand.hpp"
#include "CLI/Subcommands/ParseCsvSubcommand.hpp"
#include "CLI/Subcommands/QuerysetStatsSubcommand.hpp"
#include "CLI/Subcommands/RandomWalkSubcommand.hpp"
#include "CLI/Subcommands/SearchingSubcommand.hpp"
#include "CLI/Validators.hpp"
#include "Enums/CommandType.hpp"

int main(int argc, char **argv) {
    CLI::App app{"Run MULISSE"};

    app.require_subcommand(1);

    auto common_opts = std::make_unique<CommonOptions>();
    app.add_option("--seed", common_opts->m_seed, "Random seed")->capture_default_str();
    app.add_option("--logs", common_opts->m_logs_path, "Path to write logs to")->capture_default_str();
    app.add_option("--data", common_opts->m_data_path, "Path to the data directory")->capture_default_str();
    app.add_flag("--raw,!--normalize", common_opts->m_raw, "Do not normalize");

    umap<CommandType, uptr<ISubcommand>> subcommands{
        {CREATE_DS, std::make_unique<RandomWalkSubcommand>(app)},
        {PARSE_CSV, std::make_unique<ParseCsvSubcommand>(app)},
        {CALC_D_STATS, std::make_unique<DatasetStatsSubcommand>(app)},
        {CREATE_QS, std::make_unique<CreateQueriesSubcommand>(app)},
        {CALC_Q_STATS, std::make_unique<QuerysetStatsSubcommand>(app)},
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

    auto selected_command = std::move(subcommands[selected_command_type]);
    selected_command->set_up_execution(common_opts.get());
    selected_command->validate_arguments();
    selected_command->execute();

    return 0;
}

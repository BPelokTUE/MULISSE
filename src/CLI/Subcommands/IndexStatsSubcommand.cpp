#include "CLI/Subcommands/IndexStatsSubcommand.hpp"

#include "Enums/CommandType.hpp"
#include "Modules/CalcIndexStats.hpp"

IndexStatsSubcommand::IndexStatsSubcommand(CLI::App &app) {
    auto i_stats_subcommand = app.add_subcommand(CMD_TYPE_TO_STR.at(CALC_I_STATS), "Calculate index statistics");

    i_stats_subcommand->add_option("-i,--index_meta", m_index_meta_path, "Index file path relative to `DATA`")
        ->required();
    i_stats_subcommand->add_flag("--separate_segment_stats", m_separate_segment_stats,
                                 "Calculate segment statistics for each segment separately");
}

void IndexStatsSubcommand::execute() {
    // Set up run setting properties
    // Get data from meta file
    IndexType index_type = ENVELOPE;
    ArchiveType index_format = BINARY;
    uint num_l_groups = 1;
    calculate_index_stats(index_type, num_l_groups, index_format, m_separate_segment_stats);
}

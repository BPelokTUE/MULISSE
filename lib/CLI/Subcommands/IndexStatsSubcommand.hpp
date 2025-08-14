#ifndef CLI_SUBCOMMANDS_INDEXSTATSSUBCOMMAND_HPP
#define CLI_SUBCOMMANDS_INDEXSTATSSUBCOMMAND_HPP

#include "CLI/Subcommands/Subcommand.hpp"
#include "CLI11/CLI11.hpp"
#include "Util/Types/String.hpp"

class IndexStatsSubcommand : public ISubcommand {
   public:
    /**
     * @brief Constructor
     * @param app The CLI application to add the subcommand to
     */
    IndexStatsSubcommand(CLI::App &app);

    void execute() override;

   private:
    bool m_separate_segment_stats;
    str m_index_meta_path;
};

#endif  // CLI_SUBCOMMANDS_INDEXSTATSSUBCOMMAND_HPP

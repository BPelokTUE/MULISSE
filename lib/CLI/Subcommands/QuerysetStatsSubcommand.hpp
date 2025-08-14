#ifndef CLI_SUBCOMMANDS_QUERYSETSTATSSUBCOMMAND_HPP
#define CLI_SUBCOMMANDS_QUERYSETSTATSSUBCOMMAND_HPP

#include "CLI/Subcommands/Subcommand.hpp"
#include "CLI11/CLI11.hpp"
#include "Util/Types/String.hpp"

class QuerysetStatsSubcommand : public ISubcommand {
   public:
    /**
     * @brief Constructor
     * @param app The CLI application to add the subcommand to
     */
    QuerysetStatsSubcommand(CLI::App &app);

    void execute() override;

   private:
    str m_dataset_meta_path, m_query_meta_path;
};

#endif  // CLI_SUBCOMMANDS_QUERYSETSTATSSUBCOMMAND_HPP

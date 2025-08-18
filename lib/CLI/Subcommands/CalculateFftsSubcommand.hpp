#ifndef CLI_SUBCOMMANDS_CALCULATEFFTSSUBCOMMAND_HPP
#define CLI_SUBCOMMANDS_CALCULATEFFTSSUBCOMMAND_HPP

#include "CLI/Subcommands/Subcommand.hpp"
#include "CLI11/CLI11.hpp"
#include "Util/Types/String.hpp"

class CalculateFftsSubcommand : public ISubcommand {
   public:
    /**
     * @brief Constructor
     * @param app The CLI application to add the subcommand to
     */
    CalculateFftsSubcommand(CLI::App &app);

    void execute() override;

   private:
    str m_dataset_meta_path, m_ffts_path;
};

#endif  // CLI_SUBCOMMANDS_CALCULATEFFTSSUBCOMMAND_HPP

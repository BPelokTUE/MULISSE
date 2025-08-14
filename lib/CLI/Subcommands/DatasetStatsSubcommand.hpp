#ifndef CLI_SUBCOMMANDS_DATASETSTATSSUBCOMMAND_HPP
#define CLI_SUBCOMMANDS_DATASETSTATSSUBCOMMAND_HPP

#include "CLI/Subcommands/Subcommand.hpp"
#include "CLI11/CLI11.hpp"
#include "Util/Types/Numbers.hpp"
#include "Util/Types/String.hpp"

class DatasetStatsSubcommand : public ISubcommand {
   public:
    /**
     * @brief Constructor
     * @param app The CLI application to add the subcommand to
     */
    DatasetStatsSubcommand(CLI::App &app);

    void set_up_execution(const CommonOptions *common_opts) override;

    void execute() override;

   private:
    str m_dataset_meta_path;
    uint m_num_lags;
};

#endif  // CLI_SUBCOMMANDS_DATASETSTATSSUBCOMMAND_HPP

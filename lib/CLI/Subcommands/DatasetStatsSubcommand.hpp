#ifndef CLI_SUBCOMMANDS_DATASETSTATSSUBCOMMAND_HPP
#define CLI_SUBCOMMANDS_DATASETSTATSSUBCOMMAND_HPP

#include "CLI/Subcommands/Subcommand.hpp"
#include "CLI11/CLI11.hpp"
#include "Util/Artefacts/MtsDataset.hpp"
#include "Util/Types/Numbers.hpp"
#include "Util/Types/String.hpp"

class DatasetStatsSubcommand : public ISubcommand {
   public:
    /**
     * @brief Constructor
     * @param app The CLI application to add the subcommand to
     */
    DatasetStatsSubcommand(CLI::App &app);

    void execute(const RunContext &run_context) override;

   private:
    uint m_num_lags;
    str m_dataset_meta_path;
    MtsDataset m_mts_dataset;
};

#endif  // CLI_SUBCOMMANDS_DATASETSTATSSUBCOMMAND_HPP

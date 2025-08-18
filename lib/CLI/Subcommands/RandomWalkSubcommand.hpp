#ifndef CLI_SUBCOMMANDS_RANDOMWALKSUBCOMMAND_HPP
#define CLI_SUBCOMMANDS_RANDOMWALKSUBCOMMAND_HPP

#include "CLI/Subcommands/Subcommand.hpp"
#include "CLI11/CLI11.hpp"
#include "Util/Artefacts/Options/RandomWalkGenOptions.hpp"
#include "Util/Artefacts/Properties/MtsDatasetProperties.hpp"
#include "Util/Types/Numbers.hpp"

class RandomWalkSubcommand : public ISubcommand {
   public:
    /**
     * @brief Constructor
     * @param app The CLI application to add the subcommand to
     */
    RandomWalkSubcommand(CLI::App &app);

    void execute(const RunContext &run_context) override;

   private:
    RandomWalkGenOptions m_rw_gen_opts;
    MtsDatasetProperties m_dataset_props;
};

#endif  // CLI_SUBCOMMANDS_RANDOMWALKSUBCOMMAND_HPP

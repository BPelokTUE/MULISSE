#ifndef CLI_SUBCOMMANDS_RANDOMWALKSUBCOMMAND_HPP
#define CLI_SUBCOMMANDS_RANDOMWALKSUBCOMMAND_HPP

#include "CLI/Subcommands/Subcommand.hpp"
#include "CLI11/CLI11.hpp"
#include "Util/Artefacts/Settings/MtsDatasetSettings.hpp"
#include "Util/Types/Numbers.hpp"

class RandomWalkSubcommand : public ISubcommand {
   public:
    /**
     * @brief Constructor
     * @param app The CLI application to add the subcommand to
     */
    RandomWalkSubcommand(CLI::App &app);

    virtual void set_up_execution(const CommonOptions *common_opts) override;

    void execute() override;

   private:
    bool m_zero_start;
    Real m_step_sd;
    MtsDatasetSettings m_settings;
};

#endif  // CLI_SUBCOMMANDS_RANDOMWALKSUBCOMMAND_HPP

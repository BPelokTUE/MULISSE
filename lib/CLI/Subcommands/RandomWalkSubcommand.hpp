#ifndef CLI_SUBCOMMANDS_RANDOMWALKSUBCOMMAND_HPP
#define CLI_SUBCOMMANDS_RANDOMWALKSUBCOMMAND_HPP

#include "CLI/Subcommands/Subcommand.hpp"
#include "CLI11/CLI11.hpp"
#include "Util/Types/Containers.hpp"
#include "Util/Types/Numbers.hpp"

class RandomWalkSubcommand : public ISubcommand {
   public:
    /**
     * @brief Constructor
     * @param app The CLI application to add the subcommand to
     */
    RandomWalkSubcommand(CLI::App &app);

    void execute() override;

   private:
    str m_dataset_path;
    Real m_step_sd;
    bool m_zero_start;
    uint m_num_series, m_series_len;
    MtsNumChannels m_num_channels;
};

#endif CLI_SUBCOMMANDS_RANDOMWALKSUBCOMMAND_HPP

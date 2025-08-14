#ifndef CLI_SUBCOMMANDS_PARSECSVSUBCOMMAND_HPP
#define CLI_SUBCOMMANDS_PARSECSVSUBCOMMAND_HPP

#include "CLI/Subcommands/Subcommand.hpp"
#include "CLI11/CLI11.hpp"
#include "Util/Artefacts/Properties/MtsDatasetProperties.hpp"
#include "Util/Types/Numbers.hpp"
#include "Util/Types/String.hpp"
#include "Util/Types/Vec.hpp"

class ParseCsvSubcommand : public ISubcommand {
   public:
    /**
     * @brief Constructor
     * @param app The CLI application to add the subcommand to
     */
    ParseCsvSubcommand(CLI::App &app);

    void set_up_execution(const CommonOptions *common_opts) override;

    void validate_arguments() override;

    void execute() override;

   private:
    vec<str> m_csv_paths;
    uint m_l_min, m_l_max;
    Real m_min_subs_sd;
    MtsDatasetProperties m_dataset_props;
};

#endif  // CLI_SUBCOMMANDS_PARSECSVSUBCOMMAND_HPP

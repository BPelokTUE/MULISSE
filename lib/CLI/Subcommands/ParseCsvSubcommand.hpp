#ifndef CLI_SUBCOMMANDS_PARSECSVSUBCOMMAND_HPP
#define CLI_SUBCOMMANDS_PARSECSVSUBCOMMAND_HPP

#include "CLI/Subcommands/Subcommand.hpp"
#include "CLI11/CLI11.hpp"
#include "Util/Artefacts/Options/CsvDatasetGenOptions.hpp"
#include "Util/Artefacts/Properties/MtsDatasetProperties.hpp"
#include "Util/Types/LengthRange.hpp"
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

    void execute(const RunContext &run_context) override;

   private:
    CsvDatasetGenOptions m_csv_gen_opts;
    MtsDatasetProperties m_dataset_props;
};

#endif  // CLI_SUBCOMMANDS_PARSECSVSUBCOMMAND_HPP

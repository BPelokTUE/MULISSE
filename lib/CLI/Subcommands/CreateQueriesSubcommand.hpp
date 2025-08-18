#ifndef CLI_SUBCOMMANDS_CREATEQUERIESSUBCOMMAND_HPP
#define CLI_SUBCOMMANDS_CREATEQUERIESSUBCOMMAND_HPP

#include "CLI/Subcommands/Subcommand.hpp"
#include "CLI11/CLI11.hpp"
#include "Util/Artefacts/MtsDataset.hpp"
#include "Util/Artefacts/Options/QuerySetGenOptions.hpp"
#include "Util/Artefacts/Properties/MtsQuerySetProperties.hpp"
#include "Util/Types/Numbers.hpp"
#include "Util/Types/String.hpp"

class CreateQueriesSubcommand : public ISubcommand {
   public:
    /**
     * @brief Constructor
     * @param app The CLI application to add the subcommand to
     */
    CreateQueriesSubcommand(CLI::App &app);

    void set_up_execution(const RunContext *run_context) override;

    void validate_arguments() override;

    void execute() override;

   private:
    str m_dataset_meta_path, m_query_set_path;
    MtsDataset m_dataset;
    MtsQuerySetProperties m_query_set_props;
    QuerySetGenOptions m_query_gen_opts;
};

#endif  // CLI_SUBCOMMANDS_CREATEQUERIESSUBCOMMAND_HPP

#ifndef CLI_SUBCOMMANDS_CREATEQUERIESSUBCOMMAND_HPP
#define CLI_SUBCOMMANDS_CREATEQUERIESSUBCOMMAND_HPP

#include "CLI/Subcommands/Subcommand.hpp"
#include "CLI11/CLI11.hpp"
#include "Search/QuerySetOptions.hpp"
#include "Util/Types/Containers.hpp"
#include "Util/Types/Numbers.hpp"

class CreateQueriesSubcommand : public ISubcommand {
   public:
    /**
     * @brief Constructor
     * @param app The CLI application to add the subcommand to
     */
    CreateQueriesSubcommand(CLI::App &app);

    void set_up_execution(const CommonOptions *common_opts) override;

    void validate_arguments() override;

    void execute() override;

   private:
    QuerySetOptions m_queryset_opts;
    str m_dataset_meta_path, m_queryset_path;
};

#endif CLI_SUBCOMMANDS_CREATEQUERIESSUBCOMMAND_HPP

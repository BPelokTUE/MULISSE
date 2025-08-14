#include "CLI/Subcommands/Subcommand.hpp"

#include "CLI/CommonOptions.hpp"

void ISubcommand::set_up_execution(const CommonOptions *common_opts) { m_common_opts = common_opts; }

void ISubcommand::validate_arguments() {}

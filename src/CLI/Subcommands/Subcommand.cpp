#include "CLI/Subcommands/Subcommand.hpp"

#include "Util/Types/RunContext.hpp"

void ISubcommand::set_up_execution(const RunContext *common_opts) { m_run_context = common_opts; }

void ISubcommand::validate_arguments() {}

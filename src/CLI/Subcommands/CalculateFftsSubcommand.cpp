#include "CLI/Subcommands/CalculateFftsSubcommand.hpp"

#include <filesystem>

#include "CLI/CommonOptions.hpp"
#include "CLI/Validators.hpp"
#include "Enums/CommandType.hpp"
#include "Modules/CalcFfts.hpp"
#include "Util/HelperFuncs/Path.hpp"

namespace fs = std::filesystem;

CalculateFftsSubcommand::CalculateFftsSubcommand(CLI::App &app) {
    auto ffts_subcommand = app.add_subcommand(CMD_TYPE_TO_STR.at(CALC_FFTS), "Calculate FFTs");
    ffts_subcommand->add_option("-d,--dataset", m_dataset_meta_path, "Path to the dataset meta file")
        ->required()
        ->check(validators::file_is_readable);
    ffts_subcommand->add_option("-F,--ffts", m_ffts_path, "Path to save FFTs")
        ->required()
        ->check(validators::file_is_writable);
}

void CalculateFftsSubcommand::set_up_execution(const CommonOptions *common_opts) {
    // Set up run setting properties
}

void CalculateFftsSubcommand::execute() { calculate_ffts(!m_common_opts->m_raw); }

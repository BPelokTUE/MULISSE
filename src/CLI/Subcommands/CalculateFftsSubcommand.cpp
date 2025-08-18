#include "CLI/Subcommands/CalculateFftsSubcommand.hpp"

#include <filesystem>

#include "CLI/Validators.hpp"
#include "Enums/CommandType.hpp"
#include "Modules/CalcFfts.hpp"
#include "Util/Artefacts/MtsDataset.hpp"
#include "Util/Artefacts/MtsFfts.hpp"
#include "Util/HelperFuncs/Path.hpp"
#include "Util/Types/RunContext.hpp"

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

void CalculateFftsSubcommand::execute() {
    MtsDataset dataset;
    dataset.load_meta(m_dataset_meta_path);
    MtsFfts ffts(m_ffts_path);

    calculate_ffts(ffts, dataset, *m_run_context);
    ffts.save_meta(ffts.get_meta_path());
}

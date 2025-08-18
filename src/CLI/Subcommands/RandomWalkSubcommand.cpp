#include "CLI/Subcommands/RandomWalkSubcommand.hpp"

#include <filesystem>

#include "CLI/Validators.hpp"
#include "Enums/CommandType.hpp"
#include "Modules/RandomWalk.hpp"
#include "Util/Artefacts/MtsDataset.hpp"
#include "Util/Logging/DatasetLogger.hpp"
#include "Util/Types/RunContext.hpp"

RandomWalkSubcommand::RandomWalkSubcommand(CLI::App &app) {
    auto rw_subcommand = app.add_subcommand(CMD_TYPE_TO_STR.at(CREATE_DS), "Create random walk dataset");

    rw_subcommand->add_option("-d,--dataset", m_dataset_props.m_dataset_path, "Output dataset path")->required();
    rw_subcommand->add_option("-s,--step_sd", m_rw_gen_opts.m_step_sd, "Random walk step standard deviation")
        ->check(validators::positive_real)
        ->capture_default_str();
    rw_subcommand->add_flag("-z,--zero_start", m_rw_gen_opts.m_zero_start, "Start the random walk from zero");
    rw_subcommand->add_option("-n,--num_series", m_dataset_props.m_num_series, "Number of series")
        ->required()
        ->check(validators::positive_int);
    rw_subcommand->add_option("-m,--series_len", m_dataset_props.m_series_len, "Length of series")
        ->required()
        ->check(validators::positive_int);
    rw_subcommand->add_option("-c,--num_channels", m_dataset_props.m_num_channels, "Number of channels")
        ->required()
        ->check(validators::positive_int);
}

void RandomWalkSubcommand::execute(const RunContext &run_context) {
    // Set up dataset generation
    MtsDataset dataset(m_dataset_props);
    dataset.set_up_generation(run_context.m_data_path);

    m_rw_gen_opts.m_seed = run_context.m_seed;

    DatasetLogger logger(run_context.m_logs_path);

    // Generate time series
    create_random_walks(dataset, m_rw_gen_opts, logger);

    // Save dataset meta
    dataset.save_meta(dataset.get_meta_path());
}

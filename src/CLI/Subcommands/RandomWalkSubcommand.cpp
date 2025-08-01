#include "CLI/Subcommands/RandomWalkSubcommand.hpp"

#include "CLI/CommonOptions.hpp"
#include "CLI/Validators.hpp"
#include "Enums/CommandType.hpp"
#include "Modules/RandomWalk.hpp"

RandomWalkSubcommand::RandomWalkSubcommand(CLI::App &app) {
    auto rw_subcommand = app.add_subcommand(CMD_TYPE_TO_STR.at(CREATE_DS), "Create random walk dataset");
    const auto &positive_int = validators::get_instance().get_positive_int_validator();

    rw_subcommand->add_option("-d,--dataset", m_dataset_path, "Output dataset path relative to `DATA`")->required();
    rw_subcommand->add_option("-s,--step_sd", m_step_sd, "Random walk step standard deviation")->capture_default_str();
    rw_subcommand->add_flag("-z,--zero_start", m_zero_start, "Start the random walk from zero");
    rw_subcommand->add_option("-n,--num_series", m_num_series, "Number of series")->required()->check(positive_int);
    rw_subcommand->add_option("-m,--series_len", m_series_len, "Length of series")->required()->check(positive_int);
    rw_subcommand->add_option("-c,--num_channels", m_num_channels, "Number of channels")
        ->required()
        ->check(positive_int);
}

void RandomWalkSubcommand::execute(const CommonOptions &common_opts) {
    // Set up run properties
    create_random_walks(m_step_sd, m_zero_start, common_opts.m_seed);
}

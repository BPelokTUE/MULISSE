#include "Util/Logging/ParamEstimatesLogger.hpp"

#include "Index/EnvelopeIndex/Flat/FlatEnvelopeParams.hpp"
#include "Util/RunSettings/RunSettings.hpp"

namespace fs = std::filesystem;

ParamEstimatesLogger ParamEstimatesLogger::instance = ParamEstimatesLogger();
bool ParamEstimatesLogger::initialized = false;

void ParamEstimatesLogger::initialize() {
#ifndef DISABLE_LOGGING
    if (initialized) return;
    initialized = true;

    ParamEstimatesLogger &instance = get_instance();

    auto &RS = RunSettings::get_instance();
    instance.m_param_estimate_file_path = fs::path(RS.get_logs_path()) / PARAM_ESTIMATE_FILE;
    instance.file_setup(instance.m_param_estimate_file_path, PARAM_ESTIMATES_COL_STRS);

    instance.m_estimate_id = instance.determine_index(instance.m_param_estimate_file_path);
    instance.m_index_file = RS.m_index_file;
#endif  // DISABLE_LOGGING
}

void ParamEstimatesLogger::write_entry(const FlatEnvelopeParams &params, Real score) {
#ifndef DISABLE_LOGGING
    write_row(m_param_estimate_file_path,
              {
                  {PEC::ID, to_string(m_estimate_id++)},
                  {PEC::INDEX_FILE, m_index_file},
                  {PEC::L_PER_GROUP, format_num_param(params.m_l_per_group)},
                  {PEC::POS_PER_ENV, to_string(params.m_pos_per_env)},
                  {PEC::NUM_SEGMENTS, to_string(params.m_num_segments)},
                  {PEC::SCORE, to_string(score)},
              },
              PARAM_ESTIMATES_COL_ENUMS);
#endif  // DISABLE_LOGGING
}

const str ParamEstimatesLogger::PARAM_ESTIMATE_FILE = "param_estimates.csv";

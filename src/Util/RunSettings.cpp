#include <fstream>
#include <filesystem>
#include <memory>

#include "Search/Options/IndexOptions.hpp"
#include "Util/RunSettings.hpp"
#include "Util/typedefs.hpp"
#include "Util/FftArray.hpp"

namespace fs = std::filesystem;

// Initialize static members
sptr<RunSettings> RunSettings::instance = std::make_shared<RunSettings>();
bool RunSettings::initialized = false;
RunSettings::RunSettings() {}

void check_path_exists(str path, str name) {
    if (!fs::exists(path)) {
        throw std::runtime_error(name + " file " + path + " does not exist");
    }
}

void RunSettings::initialize(CommandType command_type, DatasetProperties dataset_props, QueryProperties query_props,
                             uint pos_per_env, const str index_path, const str ffts_path,
                             SearchMethodType method_type) {
    if (initialized) return;
    initialized = true;

    // Create directories if they do not exist
    if (!fs::exists(instance->DATA_DIR)) fs::create_directories(instance->DATA_DIR);
#ifndef DISABLE_LOGGING
    if (!fs::exists(instance->LOGS_DIR)) fs::create_directories(instance->LOGS_DIR);
#endif

    instance->m_command_type = command_type;
    instance->m_dataset_props = dataset_props;
    if (dataset_props.num_series == 0 && !dataset_props.file.empty()) {
        size_t dataset_size = get_dataset_size(instance->get_dataset_path());
        instance->m_dataset_props.num_series =
            dataset_size / (dataset_props.series_len * dataset_props.num_channels * sizeof(float));
    }

    instance->m_query_properties = query_props;

    uint envs_per_ts =
        pos_per_env == 0 ? 1 : (dataset_props.series_len - query_props.l_min + pos_per_env) / pos_per_env;
    instance->m_envelope_props = {
        .pos_per_env = pos_per_env,
        .envs_per_ts = envs_per_ts,
    };

    instance->m_index_file = index_path;
    instance->m_ffts_file = ffts_path;
    instance->m_ffts_supported =
        instance->m_ffts_file != "" && (method_type == ISAX_ENVELOPE || method_type == ENVELOPE);

    switch (instance->m_command_type) {
        case CREATE_DS:
            break;
        case PARSE_CSV:
            break;
        case CREATE_QS:
            check_path_exists(instance->get_dataset_path(), "Dataset");
            break;
        case CALC_Q_STATS:
            check_path_exists(instance->get_dataset_path(), "Dataset");
            check_path_exists(instance->get_query_path(), "Query");
            break;
        case INDEX:
            check_path_exists(instance->get_dataset_path(), "Dataset");
            if (instance->ffts_supported() && envs_per_ts > 1) {
                throw std::runtime_error(
                    "Precalculating FFTs are only supported for setups with one envelope per time series");
            }
            break;
        case CALC_I_STATS:
            check_path_exists(instance->get_index_path(), "Index");
            break;
        case CALC_FFTS:
            check_path_exists(instance->get_dataset_path(), "Dataset");
            break;
        case SEARCH:
            check_path_exists(instance->get_dataset_path(), "Dataset");
            check_path_exists(instance->get_query_path(), "Query");
            if (instance->ffts_supported()) {
                check_path_exists(instance->get_ffts_path(), "FFTs");
                instance->m_ffts_ifs.open(instance->get_ffts_path(), std::ios::binary);
                instance->m_query_ffts.resize(instance->m_dataset_props.num_channels);
                for (auto &channel_ffts : instance->m_query_ffts) channel_ffts = nullptr;
            }
            break;
    }
}

RunSettings &RunSettings::get_instance() {
    assert(initialized);
    return *instance.get();
}

#ifdef ENABLE_TEST_CODE
void RunSettings::set_instance(sptr<RunSettings> instance) {
    RunSettings::instance = instance;
    initialized = true;
}
#endif

// FFTs

void RunSettings::calculate_ffts() const {
    if (!ffts_supported()) return;

    std::ifstream ifs(get_dataset_path(), std::ios::binary);
    if (!ifs.is_open()) {
        throw std::runtime_error("Could not open dataset file for FFT calculation");
    }
    std::ofstream ofs(get_ffts_path(), std::ios::binary);
    if (!ofs.is_open()) {
        throw std::runtime_error("Could not open FFTs file for writing");
    }

    fftw_plan plan;
    uint num_chunks = m_dataset_props.num_series * m_dataset_props.num_channels;
    for (uint i = 0; i < num_chunks; ++i) {
        vec<float> channel(m_dataset_props.series_len);
        ifs.read(reinterpret_cast<char *>(channel.data()), m_dataset_props.series_len * sizeof(float));

        uint fft_len = 2 * m_dataset_props.series_len;
        FftArray channel_complex(fft_len), channel_ffts(fft_len);
        for (uint j = 0; j < m_dataset_props.series_len; ++j) channel_complex[j][0] = channel[j];

        plan = fftw_plan_dft_1d(fft_len, channel_complex.data(), channel_ffts.data(), FFTW_FORWARD, FFTW_ESTIMATE);

        fftw_execute(plan);
        fftw_destroy_plan(plan);

        for (uint j = 0; j < fft_len; ++j) {
            auto &fft = channel_ffts[j];
            ofs.write(reinterpret_cast<const char *>(&fft[0]), sizeof(double));
            ofs.write(reinterpret_cast<const char *>(&fft[1]), sizeof(double));
        }
    }
}

FftArray RunSettings::get_ffts(SubsequenceInfo subs_info, MtsNumChannelsT channel_ind, uint num_component) {
    if (!ffts_supported()) throw std::runtime_error("FFTs are not supported");

    // (*2) for using double instead of float
    // (*2) for real and imaginary parts
    // (*2) for extra components at the end
    uint file_size_ratio = 8;
    size_t data_file_pos =
        subs_info.get_file_pos(m_dataset_props.series_len, m_dataset_props.num_channels, channel_ind);
    m_ffts_ifs.seekg(file_size_ratio * data_file_pos);

    FftArray ffts(2 * num_component);
    m_ffts_ifs.read(reinterpret_cast<char *>(ffts.data()), 4 * num_component * sizeof(double));

    return ffts;
}

bool RunSettings::ffts_supported() const { return m_ffts_supported; }

void RunSettings::calculate_query_ffts(const vec<DistanceT> &q_channel, MtsNumChannelsT channel_ind,
                                       uint num_components) {
    if (!ffts_supported()) return;

    assert(channel_ind < m_dataset_props.num_channels);

    uint fft_len = 2 * num_components, query_len = q_channel.size();
    FftArray q_complex(fft_len);
    for (uint i = 0; i < query_len; ++i) q_complex[i][0] = q_channel[query_len - 1 - i];

    m_query_ffts[channel_ind] = std::make_unique<FftArray>(fft_len);
    fftw_plan plan =
        fftw_plan_dft_1d(fft_len, q_complex.data(), m_query_ffts[channel_ind]->data(), FFTW_FORWARD, FFTW_ESTIMATE);

    fftw_execute(plan);
    fftw_destroy_plan(plan);
}

const FftArray *RunSettings::get_query_ffts(MtsNumChannelsT channel_ind) const {
    if (!ffts_supported()) throw std::runtime_error("FFTs are not supported");

    assert(channel_ind < m_dataset_props.num_channels);

    return m_query_ffts[channel_ind].get();
}

void RunSettings::reset_query_ffts() {
    if (!ffts_supported()) return;

    for (MtsNumChannelsT c = 0; c < m_dataset_props.num_channels; ++c) m_query_ffts[c] = nullptr;
}

// iSAX

const vec<float> &RunSettings::get_breakpoints() { return m_isax_props.breakpoints; }

void RunSettings::update_breakpoints() {
    auto &breakpoint_strategy = m_isax_props.breakpoint_strategy;
    m_isax_props.breakpoints = breakpoint_strategy->get_breakpoints(1 << m_isax_props.breakpoint_num_bits);
}

void RunSettings::set_isax_properties(iSaxProperties isax_props) {
    if (!m_isax_props_set) {
        m_isax_props = std::move(isax_props);
        m_isax_props_set = true;
    }
}

// Properties

const DatasetProperties &RunSettings::get_dataset_props() { return m_dataset_props; }

const QueryProperties &RunSettings::get_query_props() { return m_query_properties; }

const iSaxProperties &RunSettings::get_isax_props() { return m_isax_props; }

// Paths

str RunSettings::get_dataset_path() const { return fs::path(DATA_DIR) / m_dataset_props.file; }

str RunSettings::get_query_path() const { return fs::path(DATA_DIR) / m_query_properties.file; }

str RunSettings::get_index_path() const { return m_index_file.empty() ? "" : fs::path(DATA_DIR) / m_index_file; }

str RunSettings::get_ffts_path() const { return m_ffts_file.empty() ? "" : fs::path(DATA_DIR) / m_ffts_file; }

str RunSettings::get_logs_path() const { return LOGS_DIR; }

// Load index

std::ifstream RunSettings::get_index_ifs() const {
    str index_path = get_index_path();
    std::ifstream index_stream(index_path, std::ios::binary);
    if (!index_stream) {
        std::cerr << "Could not open index file " << index_path << std::endl;
        return std::ifstream();
    }
    return index_stream;
}

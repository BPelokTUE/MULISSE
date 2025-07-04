#include "Util/RunSettings/RunSettings.hpp"

#include <filesystem>
#include <fstream>
#include <memory>

#include "Index/EnvelopeIndex/Flat/FlatEnvelopeParams.hpp"
#include "Util/HelperFuncs/Conversion.hpp"
#include "Util/HelperFuncs/Path.hpp"

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

void RunSettings::initialize(CommandType command_type, DatasetProperties dataset_props, LengthProperties length_props,
                             uint pos_per_env, const str &index_path, const str &ffts_path, const str &query_path,
                             SearchMethodType method_type, const str &logs_dir, const str &data_dir) {
    if (initialized) return;
    initialized = true;

    // Create directories if they do not exist
    instance->m_data_dir = data_dir;
    if (!fs::exists(instance->m_data_dir)) fs::create_directories(instance->m_data_dir);
    instance->m_logs_dir = logs_dir;
#ifndef DISABLE_LOGGING
    if (!fs::exists(instance->m_logs_dir)) fs::create_directories(instance->m_logs_dir);
#endif

    instance->m_command_type = command_type;
    instance->m_dataset_props = dataset_props;
    if (dataset_props.m_num_series == 0 && !dataset_props.m_file.empty()) {
        size_t dataset_size = get_dataset_size(instance->get_dataset_path());
        instance->m_dataset_props.m_num_series =
            U(dataset_size / (dataset_props.m_series_len * dataset_props.m_num_channels * sizeof(Real)));
    }

    instance->m_length_props = length_props;
    if (!length_props.m_use_length_groups) {
        instance->m_length_props.m_num_l_groups = 1;
        instance->m_length_props.m_l_per_group = length_props.m_l_max - length_props.m_l_min + 1;
    } else {
        instance->set_lengths_per_group(length_props.m_l_per_group);
    }

    instance->set_pos_per_env(pos_per_env);

    instance->m_index_file = index_path;
    instance->m_ffts_file = ffts_path;
    instance->m_query_file = query_path;
    instance->m_ffts_supported = !(instance->m_ffts_file.empty());

    switch (instance->m_command_type) {
        case CREATE_DS:
        case PARSE_CSV:
            break;
        case CALC_D_STATS:
        case CREATE_QS:
            check_path_exists(instance->get_dataset_path(), "Dataset");
            break;
        case CALC_Q_STATS:
            check_path_exists(instance->get_dataset_path(), "Dataset");
            check_path_exists(instance->get_query_path(), "Query");
            break;
        case INDEX:
            check_path_exists(instance->get_dataset_path(), "Dataset");
            break;
        case CALC_I_STATS:
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
                instance->m_query_ffts.resize(instance->m_dataset_props.m_num_channels);
                for (auto &channel_ffts : instance->m_query_ffts) channel_ffts = nullptr;
            }
            break;
    }
}

#ifdef ENABLE_TEST_CODE
void RunSettings::set_instance(sptr<RunSettings> instance_) {
    RunSettings::instance = instance_;
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

    fftwr_plan plan;
    uint num_chunks = m_dataset_props.m_num_series * m_dataset_props.m_num_channels;
    for (uint i = 0; i < num_chunks; ++i) {
        vec<Real> channel(m_dataset_props.m_series_len);
        ifs.read(reinterpret_cast<char *>(channel.data()), m_dataset_props.m_series_len * sizeof(Real));

        uint fft_len = 2 * m_dataset_props.m_series_len;
        FftArray channel_complex(fft_len), channel_ffts(fft_len);
        for (uint j = 0; j < m_dataset_props.m_series_len; ++j) channel_complex[j][0] = static_cast<MassT>(channel[j]);

        plan = fftwr_plan_dft_1d(static_cast<int>(fft_len), channel_complex.data(), channel_ffts.data(), FFTW_FORWARD,
                                 FFTW_ESTIMATE);

        fftwr_execute(plan);
        fftwr_destroy_plan(plan);

        for (uint j = 0; j < fft_len; ++j) {
            auto &fft = channel_ffts[j];
            FftPrecT real = static_cast<FftPrecT>(fft[0]), imag = static_cast<FftPrecT>(fft[1]);
            ofs.write(reinterpret_cast<const char *>(&real), sizeof(FftPrecT));
            ofs.write(reinterpret_cast<const char *>(&imag), sizeof(FftPrecT));
        }
    }
}

FftArray RunSettings::get_ffts(SubsequenceInfo subs_info, MtsNumChannelsT channel_ind) {
    if (!ffts_supported()) throw std::runtime_error("FFTs are not supported");

    // (*2) for real and imaginary parts
    // (*2) for extra components at the end
    // Potentially (*2) depending on the size of FftPrecT compared to Real
    constexpr uint file_size_ratio = 4 * sizeof(FftPrecT) / sizeof(Real);

    uint series_len = m_dataset_props.m_series_len;
    size_t data_file_pos =
        static_cast<size_t>(subs_info.get_file_pos(series_len, m_dataset_props.m_num_channels, channel_ind));
    m_ffts_ifs.seekg(static_cast<std::streamsize>(file_size_ratio * data_file_pos));

    FftArray ffts(2 * series_len);
    std::streamsize data_to_read = static_cast<std::streamsize>(file_size_ratio * series_len * sizeof(Real));

    if constexpr (std::is_same_v<FftPrecT, MassT>) {
        m_ffts_ifs.read(reinterpret_cast<char *>(ffts.data()), data_to_read);
    } else {
        vec<FftPrecT> fft_prec(4 * series_len);
        m_ffts_ifs.read(reinterpret_cast<char *>(fft_prec.data()), data_to_read);
        for (uint i = 0; i < 2 * series_len; ++i) {
            ffts[i][0] = static_cast<MassT>(fft_prec[2 * i]);
            ffts[i][1] = static_cast<MassT>(fft_prec[2 * i + 1]);
        }
    }

    return ffts;
}

bool RunSettings::ffts_supported() const { return m_ffts_supported; }

void RunSettings::calculate_query_ffts(const vec<MassT> &q_channel, MtsNumChannelsT channel_ind) {
    if (!ffts_supported()) return;

    assert(channel_ind < m_dataset_props.m_num_channels);

    uint fft_len = 2 * m_dataset_props.m_series_len, query_len = U(q_channel.size());
    FftArray q_complex(fft_len);
    for (uint i = 0; i < query_len; ++i) q_complex[i][0] = q_channel[query_len - 1 - i];

    m_query_ffts[channel_ind] = std::make_unique<FftArray>(fft_len);
    fftwr_plan plan = fftwr_plan_dft_1d(static_cast<int>(fft_len), q_complex.data(), m_query_ffts[channel_ind]->data(),
                                        FFTW_FORWARD, FFTW_ESTIMATE);

    fftwr_execute(plan);
    fftwr_destroy_plan(plan);
}

const FftArray *RunSettings::get_query_ffts(MtsNumChannelsT channel_ind) const {
    if (!ffts_supported()) throw std::runtime_error("FFTs are not supported");

    assert(channel_ind < m_dataset_props.m_num_channels);

    return m_query_ffts[channel_ind].get();
}

void RunSettings::reset_query_ffts() {
    if (!ffts_supported()) return;

    for (MtsNumChannelsT c = 0; c < m_dataset_props.m_num_channels; ++c) m_query_ffts[c] = nullptr;
}

size_t RunSettings::get_ffts_size_on_disk() {
    return ffts_supported() ? static_cast<size_t>(fs::file_size(get_ffts_path())) : 0;
}

// iSAX

const vec<Real> &RunSettings::get_breakpoints() { return m_breakpoint_props.m_breakpoints; }

void RunSettings::update_breakpoints() {
    auto &breakpoint_strategy = m_breakpoint_props.m_breakpoint_strategy;
    m_breakpoint_props.m_breakpoints =
        breakpoint_strategy->get_breakpoints(static_cast<SaxSymbolT>(1 << m_breakpoint_props.m_breakpoint_num_bits));
}

bool RunSettings::breakpoints_set() const { return m_breakpoints_props_set; }

void RunSettings::set_breakpoint_props(BreakpointProperties breakpoint_props) {
    if (!m_breakpoints_props_set) {
        m_breakpoint_props = std::move(breakpoint_props);
        m_breakpoints_props_set = true;
    }
}

// Properties

const DatasetProperties &RunSettings::get_dataset_props() const { return m_dataset_props; }

const BreakpointProperties &RunSettings::get_breakpoint_props() const { return m_breakpoint_props; }

const EnvelopeProperties &RunSettings::get_envelope_props() const { return m_envelope_props; }

const LengthProperties &RunSettings::get_length_props() const { return m_length_props; }

void RunSettings::set_pos_per_env(uint pos_per_env) {
    m_envelope_props.m_pos_per_env = pos_per_env;
    m_envelope_props.m_envs_per_ts =
        pos_per_env == 0 ? 1 : (m_dataset_props.m_series_len - m_length_props.m_l_min + pos_per_env) / pos_per_env;
}

void RunSettings::set_lengths_per_group(uint l_per_group) { m_length_props.set_lengths_per_group(l_per_group); }

void RunSettings::set_flat_envelope_params(const FlatEnvelopeParams &flat_envelope_params) {
    set_pos_per_env(flat_envelope_params.m_pos_per_env);
    set_lengths_per_group(flat_envelope_params.m_l_per_group);
}

// Paths

str RunSettings::get_dataset_path() const { return fs::path(m_data_dir) / m_dataset_props.m_file; }

str RunSettings::get_query_path() const { return fs::path(m_data_dir) / m_query_file; }

str RunSettings::get_index_path() const { return m_index_file.empty() ? "" : fs::path(m_data_dir) / m_index_file; }

str RunSettings::get_ffts_path() const { return m_ffts_file.empty() ? "" : fs::path(m_data_dir) / m_ffts_file; }

str RunSettings::get_logs_path() const { return m_logs_dir; }

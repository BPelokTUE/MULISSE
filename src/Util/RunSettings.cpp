#include "Search/IndexOptions.hpp"
#include "Util/RunSettings.hpp"

// Initialize static members
RunSettings RunSettings::instance = RunSettings();
bool RunSettings::initialized = false;
RunSettings::RunSettings() {}

void RunSettings::initialize(CommandType command_type, DatasetProperties dataset_props, QueryProperties query_props,
                             uint pos_per_env, str ffts_path) {
    if (initialized) return;

    initialized = true;

    instance.m_command_type = command_type;
    instance.m_dataset_props = dataset_props;
    if (dataset_props.num_series == 0) {
        FilePositionT dataset_size = get_dataset_size(dataset_props.path);
        instance.m_dataset_props.num_series =
            dataset_size / (dataset_props.series_len * dataset_props.num_channels * sizeof(float));
    }

    instance.m_query_properties = query_props;

    uint envs_per_ts =
        pos_per_env == 0 ? 1 : (dataset_props.series_len - query_props.l_min + pos_per_env) / pos_per_env;
    instance.m_envelope_props = {
        .pos_per_env = pos_per_env,
        .envs_per_ts = envs_per_ts,
    };

    instance.m_ffts_path = ffts_path;

    switch (instance.m_command_type) {
        case CREATE_DS:
            break;
        case CREATE_QS:
            break;
        case INDEX:
            if (instance.ffts_supported() && envs_per_ts > 1) {
                throw std::runtime_error(
                    "Precalculating FFTs are only supported for setups with one envelope per time series");
            }
            break;
        case SEARCH:
            if (instance.ffts_supported()) {
                instance.m_ffts_stream.open(instance.m_ffts_path, std::ios::binary);
                instance.m_query_ffts.resize(instance.m_dataset_props.num_channels);
                for (auto &channel_ffts : instance.m_query_ffts) channel_ffts = nullptr;
            }
            break;
    }
}

RunSettings &RunSettings::get_instance() {
    assert(initialized);
    return instance;
}

// FFTs

void RunSettings::calculate_ffts() const {
    if (!ffts_supported()) return;

    std::ifstream ifs(m_dataset_props.path, std::ios::binary);
    if (!ifs.is_open()) {
        throw std::runtime_error("Could not open dataset file for FFT calculation");
    }
    std::ofstream ofs(m_ffts_path, std::ios::binary);
    if (!ofs.is_open()) {
        throw std::runtime_error("Could not open FFTs file for writing");
    }

    fftw_plan plan;
    for (uint i = 0; i < m_dataset_props.num_series; ++i) {
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

FftArray RunSettings::get_ffts(FilePositionT file_pos, MtsNumChannelsT channel_ind, uint num_component) {
    if (!ffts_supported()) throw std::runtime_error("FFTs are not supported");

    // (*2) for using double instead of float
    // (*2) for real and imaginary parts
    // (*2) for extra components at the end
    uint file_size_ratio = 8;
    FilePositionT channel_file_pos = (file_pos + channel_ind * m_dataset_props.series_len) * sizeof(float);
    m_ffts_stream.seekg(file_size_ratio * channel_file_pos);

    FftArray ffts(2 * num_component);
    m_ffts_stream.read(reinterpret_cast<char *>(ffts.data()), 4 * num_component * sizeof(double));

    return ffts;
}

bool RunSettings::ffts_supported() const { return m_ffts_path != ""; }

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

const vec<float> &RunSettings::get_breakpoints() { return m_isax_props.m_breakpoints; }

const iSaxProperties &RunSettings::get_isax_props() { return m_isax_props; }

void RunSettings::set_isax_properties(iSaxProperties isax_props) {
    if (!m_isax_props_set) {
        m_isax_props = isax_props;
        m_isax_props_set = true;
    }
}

// Properties

const DatasetProperties &RunSettings::get_dataset_props() { return m_dataset_props; }

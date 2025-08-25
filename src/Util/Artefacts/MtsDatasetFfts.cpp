#include "Util/Artefacts/MtsDatasetFfts.hpp"

#include "Util/Artefacts/MtsDataset.hpp"
#include "Util/HelperFuncs/Errors.hpp"
#include "Util/HelperFuncs/Path.hpp"
#include "Util/Types/FftArray.hpp"
#include "Util/Types/MultivariateTimeSeries.hpp"
#include "Util/Types/SubsequencePosition.hpp"

MtsDatasetFfts::MtsDatasetFfts(const str &ffts_path, MtsDataset &dataset)
    : m_ffts_path(ffts_path), m_source_dataset(dataset), m_ostream(ffts_path) {}

str MtsDatasetFfts::get_meta_path() const {
    str path = m_ffts_path;
    path.replace(path.find_last_of('.'), path.size() - path.find_last_of('.'), "_ffts_meta.json");
    return path;
}

template <typename Archive>
void MtsDatasetFfts::apply_archive(Archive &ar) {
    ar(cereal::make_nvp("log_id", m_log_id), cereal::make_nvp("source_dataset_path", m_source_dataset_path),
       cereal::make_nvp("ffts_file", m_ffts_path));
}

void MtsDatasetFfts::apply_in_archive(cereal::JSONInputArchive &ar) { apply_archive(ar); }

void MtsDatasetFfts::apply_out_archive(cereal::JSONOutputArchive &ar) { apply_archive(ar); }

str MtsDatasetFfts::get_meta_path() const { return append_to_base(m_ffts_path, "_ffts_meta"); }

void MtsDatasetFfts::generate() {
    if (!m_source_dataset) throw errors::source_dataset_not_set();
    if (!m_ostream) throw errors::output_stream_not_set();

    auto &ostream = m_ostream.get();

    fftwr_plan plan;
    auto &ds_props = m_source_dataset->get().get_properties();

    for (uint i = 0; i < ds_props.m_num_series; ++i) {
        auto mts = m_source_dataset->get().load_next_series();

        for (MtsNumChannelsT c = 0; c < ds_props.m_num_channels; ++c) {
            auto &channel = mts[c];
            uint fft_len = 2 * ds_props.m_series_len;
            FftArray channel_complex(fft_len), channel_ffts(fft_len);
            for (uint j = 0; j < ds_props.m_series_len; ++j) channel_complex[j][0] = static_cast<MassT>(channel[j]);

            plan = fftwr_plan_dft_1d(static_cast<int>(fft_len), channel_complex.data(), channel_ffts.data(),
                                     FFTW_FORWARD, FFTW_ESTIMATE);

            fftwr_execute(plan);
            fftwr_destroy_plan(plan);

            for (uint j = 0; j < fft_len; ++j) {
                auto &fft = channel_ffts[j];
                FftPrecT real = static_cast<FftPrecT>(fft[0]), imag = static_cast<FftPrecT>(fft[1]);
                ostream.write(reinterpret_cast<const char *>(&real), sizeof(FftPrecT));
                ostream.write(reinterpret_cast<const char *>(&imag), sizeof(FftPrecT));
            }
        }
    }
}

vec<FftArray> MtsDatasetFfts::load_series_ffts(uint series_ind, const vec<bool> &channel_mask) {
    if (!m_source_dataset) throw errors::source_dataset_not_set();
    if (!m_istream) throw errors::input_stream_not_set();

    auto &istream = m_istream.get();

    constexpr uint file_size_ratio = FILE_SIZE_MULTIPLIER * sizeof(FftPrecT) / sizeof(Real);

    auto &ds_props = m_source_dataset->get().get_properties();
    uint series_len = ds_props.m_series_len;
    MtsNumChannelsT num_channels = ds_props.m_num_channels;

    SubsequencePosition series_pos{.m_series = series_ind, .m_start = 0};
    size_t data_file_pos = static_cast<size_t>(series_pos.get_file_pos(series_len, num_channels, 0));

    istream.seekg(static_cast<std::streamsize>(file_size_ratio * data_file_pos));

    vec<FftArray> series_ffts(num_channels, FftArray(2 * series_len));

    for (MtsNumChannelsT c = 0; c < num_channels; ++c) {
        std::streamsize data_to_read = static_cast<std::streamsize>(file_size_ratio * series_len * sizeof(Real));
        auto &channel_ffts = series_ffts[c];

        if constexpr (std::is_same_v<FftPrecT, MassT>) {
            istream.read(reinterpret_cast<char *>(channel_ffts.data()), data_to_read);
        } else {
            vec<FftPrecT> fft_prec(FILE_SIZE_MULTIPLIER * series_len);
            istream.read(reinterpret_cast<char *>(fft_prec.data()), data_to_read);
            for (uint i = 0; i < 2 * series_len; ++i) {
                channel_ffts[i][0] = static_cast<MassT>(fft_prec[2 * i]);
                channel_ffts[i][1] = static_cast<MassT>(fft_prec[2 * i + 1]);
            }
        }
    }
    return series_ffts;
}

MtsDataset &MtsDatasetFfts::get_source_dataset() const {
    if (!m_source_dataset) {
        throw std::runtime_error("Source dataset is not set.");
    }
    return m_source_dataset->get();
}

size_t MtsDatasetFfts::get_size_on_disk() const {
    auto path = m_ostream.get_path();
    if (path) return fs::file_size(*path);

    path = m_istream.get_path();
    if (path) return fs::file_size(*path);

    return m_source_dataset->get().get_size_on_disk() * FILE_SIZE_MULTIPLIER;
}

const str &MtsDatasetFfts::get_ffts_path() const { return m_ffts_path; }

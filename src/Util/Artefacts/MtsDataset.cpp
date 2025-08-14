#include "Util/Artefacts/MtsDataset.hpp"

#include <cereal/archives/json.hpp>
#include <fstream>
#include <string>

#include "Util/Types/MultivariateTimeSeries.hpp"
#include "Util/Types/Numbers.hpp"
#include "Util/Types/Vec.hpp"

MtsDataset::MtsDataset() = default;

MtsDataset::MtsDataset(const MtsDatasetSettings &dataset_settings) : m_settings(dataset_settings) { open_ifs(); }

void MtsDataset::open_ifs() {
    m_dataset_ifs.open(m_settings.m_dataset_path, std::ios::binary);
    if (!m_dataset_ifs.is_open()) throw std::runtime_error("Could not open dataset file: " + m_settings.m_dataset_path);
}

template <typename Archive>
void MtsDataset::apply_archive(Archive &ar) {
    ar(cereal::make_nvp("settings", m_settings.m_num_channels), cereal::make_nvp("series_len", m_settings.m_series_len),
       cereal::make_nvp("num_series", m_settings.m_num_series),
       cereal::make_nvp("dataset_file", m_settings.m_dataset_path));
}

void MtsDataset::save(const str &out_file, ArchiveType) {
    std::ofstream ofs(out_file);
    cereal::JSONOutputArchive ar(ofs);

    apply_archive(ar);
}

void MtsDataset::load(const str &in_file, ArchiveType) {
    std::ifstream ifs(in_file);
    if (!ifs.is_open()) throw std::runtime_error("Could not open dataset meta file: " + in_file);
    cereal::JSONInputArchive ar(ifs);

    apply_archive(ar);
    open_ifs();
}

const MtsDatasetSettings &MtsDataset::get_settings() const { return m_settings; }

str MtsDataset::get_channel_stats_path() const {
    str path = m_settings.m_dataset_path;
    path.replace(path.find_last_of('.'), path.size() - path.find_last_of('.'), "_ch_stats.json");
    return path;
}

str MtsDataset::get_meta_path() const {
    str path = m_settings.m_dataset_path;
    path.replace(path.find_last_of('.'), path.size() - path.find_last_of('.'), "_ds_meta.json");
    return path;
}

MultivariateTimeSeries MtsDataset::load_series(uint series_index) {
    m_dataset_ifs.seekg(series_index * m_settings.m_num_channels * m_settings.m_series_len * sizeof(Real));
    load_next_series();
}

MultivariateTimeSeries MtsDataset::load_next_series() {
    vec<vec<Real>> mts_data(m_settings.m_num_channels, vec<Real>(m_settings.m_series_len));
    for (auto &channel : mts_data) {
        m_dataset_ifs.read(reinterpret_cast<char *>(channel.data()), m_settings.m_series_len * sizeof(Real));
    }
    return MultivariateTimeSeries(std::move(mts_data));
}

#include "Util/Artefacts/MtsDataset.hpp"

#include <string>

MtsDataset::MtsDataset() = default;

MtsDataset::MtsDataset(const MtsDatasetSettings &dataset_settings) : m_settings(dataset_settings) {}

const MtsDatasetSettings &MtsDataset::get_settings() const { return m_settings; }

str MtsDataset::get_channel_stats_path() const {
    str path = m_settings.m_dataset_file;
    path.replace(path.find_last_of('.'), path.size() - path.find_last_of('.'), "_ch_stats.json");
    return path;
}

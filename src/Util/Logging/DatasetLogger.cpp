#include "Util/Logging/DatasetLogger.hpp"

#include <filesystem>

#include "Util/Artefacts/MtsDataset.hpp"
#include "Util/Artefacts/Options/CsvDatasetGenOptions.hpp"
#include "Util/Artefacts/Options/RandomWalkGenOptions.hpp"
#include "Util/HelperFuncs/Path.hpp"

namespace fs = std::filesystem;

// DatasetLogger
const str DatasetLogger::DATASET_SETTINGS_FILE = "dataset_settings.csv";

using DSC = DatasetSettingsColumn;

DatasetLogger::DatasetLogger(const str &logs_path) {
    m_dataset_settings_path = fs::path(logs_path) / DATASET_SETTINGS_FILE;
    file_setup(m_dataset_settings_path, DATASET_SETTINGS_COL_STRS);
}

void DatasetLogger::write_entry(const RandomWalkGenOptions &rw_gen_opts, const MtsDataset &dataset) const {
#ifndef DISABLE_LOGGING
    uint id = determine_index(m_dataset_settings_path);
    auto [num_channels, series_len, num_series, dataset_file] = dataset.get_properties();

    write_row(m_dataset_settings_path,
              {
                  {DSC::ID, to_string(id)},
                  {DSC::DATASET_FILE, dataset_file},
                  {DSC::NUM_CHANNELS, to_string(num_channels)},
                  {DSC::SERIES_LENGTH, to_string(series_len)},
                  {DSC::NUM_SERIES, to_string(num_series)},
                  {DSC::SD, to_string(rw_gen_opts.m_step_sd)},
                  {DSC::SEED, to_string(rw_gen_opts.m_seed)},
                  {DSC::SIZE_ON_DISK_B, to_string(dataset.get_size_on_disk())},
              },
              DATASET_SETTINGS_COL_ENUMS);
#endif  // DISABLE_LOGGING
}

void DatasetLogger::write_entry(const CsvDatasetGenOptions &csv_gen_opts, const MtsDataset &dataset) const {
#ifndef DISABLE_LOGGING
    uint id = determine_index(m_dataset_settings_path);
    auto [num_channels, series_len, num_series, dataset_file] = dataset.get_properties();

    str source_csv_str = "";
    const vec<str> &source_csvs = csv_gen_opts.m_source_csvs;
    num_channels = static_cast<MtsNumChannelsT>(source_csvs.size());
    for (uint i = 0; i < source_csvs.size(); ++i) {
        source_csv_str += source_csvs[i];
        if (i < source_csvs.size() - 1) source_csv_str += ITEM_SEP;
    };

    write_row(m_dataset_settings_path,
              {
                  {DSC::ID, to_string(id)},
                  {DSC::DATASET_FILE, dataset_file},
                  {DSC::NUM_CHANNELS, to_string(num_channels)},
                  {DSC::SERIES_LENGTH, to_string(series_len)},
                  {DSC::NUM_SERIES, to_string(num_series)},
                  {DSC::MIN_SUBS_SD, to_string(csv_gen_opts.m_min_subs_sd)},
                  {DSC::SOURCE_CSVS, source_csv_str},
                  {DSC::L_MIN, to_string(csv_gen_opts.m_l_range.m_l_min)},
                  {DSC::L_MAX, to_string(csv_gen_opts.m_l_range.m_l_max)},
                  {DSC::SEED, to_string(csv_gen_opts.m_seed)},
                  {DSC::SIZE_ON_DISK_B, to_string(dataset.get_size_on_disk())},
              },
              DATASET_SETTINGS_COL_ENUMS);
#endif  // DISABLE_LOGGING
}

#include "Util/Logging/DatasetLogger.hpp"

#include <filesystem>

#include "Util/Artefacts/MtsDataset.hpp"
#include "Util/HelperFuncs/Path.hpp"

namespace fs = std::filesystem;

// DatasetLogger
RandomWalkLogAttributes::RandomWalkLogAttributes(Real noise, int seed) : m_noise(noise), m_seed(seed) {}

DatasetType RandomWalkLogAttributes::get_type() { return RANDOM_WALK; }

CsvDatasetLogAttributes::CsvDatasetLogAttributes(const vec<str> &source_csvs, uint series_generated, uint l_min,
                                                 uint l_max, Real min_subs_sd, int seed)
    : m_source_csvs(source_csvs),
      m_series_generated(series_generated),
      m_l_min(l_min),
      m_l_max(l_max),
      m_min_subs_sd(min_subs_sd),
      m_seed(seed) {}

DatasetType CsvDatasetLogAttributes::get_type() { return CSV; }

const str DatasetLogger::DATASET_SETTINGS_FILE = "dataset_settings.csv";

using DSC = DatasetSettingsColumn;

void DatasetLogger::write_entry(uptr<IDatasetLogAttributes> attributes, const MtsDataset &dataset,
                                const str &logs_path) {
#ifndef DISABLE_LOGGING
    DatasetLogger instance;

    str dataset_settings_path = fs::path(logs_path) / DatasetLogger::DATASET_SETTINGS_FILE;
    instance.file_setup(dataset_settings_path, DATASET_SETTINGS_COL_STRS);

    // Append entry
    uint id = instance.determine_index(dataset_settings_path);
    auto [num_channels, series_len, num_series, dataset_file] = dataset.get_properties();

    str sd_str = "", min_subs_sd_str = "", source_csv_str = "", l_min_str = "", l_max_str = "", seed_str = "";
    switch (attributes->get_type()) {
        case RANDOM_WALK: {
            auto *rw_attributes = static_cast<RandomWalkLogAttributes *>(attributes.get());
            sd_str = to_string(rw_attributes->m_noise);
            seed_str = to_string(rw_attributes->m_seed);
            break;
        }
        case CSV: {
            auto *csv_attributes = static_cast<CsvDatasetLogAttributes *>(attributes.get());
            num_series = csv_attributes->m_series_generated;
            const vec<str> &source_csvs = csv_attributes->m_source_csvs;
            num_channels = static_cast<MtsNumChannelsT>(source_csvs.size());
            for (uint i = 0; i < source_csvs.size(); ++i) {
                source_csv_str += source_csvs[i];
                if (i < source_csvs.size() - 1) source_csv_str += instance.ITEM_SEP;
            }
            l_min_str = to_string(csv_attributes->m_l_min);
            l_max_str = to_string(csv_attributes->m_l_max);
            min_subs_sd_str = to_string(csv_attributes->m_min_subs_sd);
            seed_str = to_string(csv_attributes->m_seed);
            break;
        }
    }

    instance.write_row(dataset_settings_path,
                       {
                           {DSC::ID, to_string(id)},
                           {DSC::DATASET_FILE, dataset_file},
                           {DSC::NUM_CHANNELS, to_string(num_channels)},
                           {DSC::SERIES_LENGTH, to_string(series_len)},
                           {DSC::NUM_SERIES, to_string(num_series)},
                           {DSC::SD, sd_str},
                           {DSC::MIN_SUBS_SD, min_subs_sd_str},
                           {DSC::SOURCE_CSVS, source_csv_str},
                           {DSC::L_MIN, l_min_str},
                           {DSC::L_MAX, l_max_str},
                           {DSC::SEED, seed_str},
                           {DSC::SIZE_ON_DISK_B, to_string(dataset.get_size_on_disk())},
                       },
                       DATASET_SETTINGS_COL_ENUMS);
#endif  // DISABLE_LOGGING
}

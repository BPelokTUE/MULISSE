#include "Util/Logging/FftsLogger.hpp"

#include "Util/Artefacts/MtsDataset.hpp"
#include "Util/Artefacts/MtsDatasetFfts.hpp"

FftsLogger::FftsLogger(const str &logs_path) {
    m_ffts_settings_path = fs::path(logs_path) / FFTS_SETTINGS_FILE;
    file_setup(m_ffts_settings_path, FFTS_SETTINGS_COL_STRS);
}

uint FftsLogger::write_entry(const MtsDatasetFfts &ffts) {
    uint id = determine_index(m_ffts_settings_path);
#ifndef DISABLE_LOGGING

    auto &dataset = ffts.get_source_dataset();

    write_row(m_ffts_settings_path,
              {
                  {FftsSettingsColumn::ID, to_string(id)},
                  {FftsSettingsColumn::DATASET_ID, to_string(dataset.get_log_id())},
                  {FftsSettingsColumn::DATASET_FILE, dataset.get_properties().m_dataset_path},
                  {FftsSettingsColumn::FFTS_FILE, ffts.get_ffts_path()},
                  {FftsSettingsColumn::CALC_TIME_S, to_string(m_calc_time_s)},
                  {FftsSettingsColumn::SIZE_ON_DISK_B, to_string(ffts.get_size_on_disk())},
              },
              FFTS_SETTINGS_COL_ENUMS);

#endif  // DISABLE_LOGGING
    return id;
}

void FftsLogger::start_timer() { m_calc_time_start = Clock::now(); }

void FftsLogger::stop_timer() {
    auto end = Clock::now();
    m_calc_time_s = Duration(end - m_calc_time_start).count();
}

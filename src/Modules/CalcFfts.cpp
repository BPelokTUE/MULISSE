#include "Modules/CalcFfts.hpp"

#include "Util/Artefacts/MtsDataset.hpp"
#include "Util/Artefacts/MtsFfts.hpp"
#include "Util/Logging/IndexLogger.hpp"
#include "Util/Types/RunContext.hpp"

void calculate_ffts(MtsFfts &ffts, const MtsDataset &dataset, const RunContext &run_context) {
    auto [num_channels, series_len, num_series, dataset_path] = dataset.get_properties();
    auto [normalized, seed, data_path, logs_path] = run_context;

    IndexLogger::initialize({
        .m_normalized = normalized,
        .m_adapt = false,
        .m_num_channels = num_channels,
        .m_index_format = ArchiveType::NONE,
        .m_l_min = 0,
        .m_l_max = 0,
        .m_series_len = series_len,
        .m_l_per_group = 0,
        .m_index_params = nullptr,
    });
    auto &logger = IndexLogger::get_instance();

    logger.start_timer(ISC::FFT_CALC_TIME_S);
    ffts.calculate(dataset);
    logger.stop_timer(ISC::FFT_CALC_TIME_S);
    logger.increment_count_col(ISC::SIZE_ON_DISK_B, ffts.get_size_on_disk());
    logger.write_entry();
}

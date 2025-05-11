#include "Modules/CalcFfts.hpp"
#include "Util/typedefs.hpp"
#include "Util/RunSettings.hpp"
#include "Util/Logging/IndexLogger.hpp"

int calculate_ffts(bool normalized) {
    auto &RS = RunSettings::get_instance();
    uint series_len = RS.get_dataset_props().m_series_len;
    MtsNumChannelsT num_channels = RS.get_dataset_props().m_num_channels;

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

    if (RS.ffts_supported()) {
        logger.start_timer(ISC::FFT_CALC_TIME_S);
        RS.calculate_ffts();
        logger.stop_timer(ISC::FFT_CALC_TIME_S);
        logger.increment_count_col(ISC::SIZE_ON_DISK_B, RS.get_ffts_size_on_disk());
    }

    logger.write_entry();

    return 0;
}

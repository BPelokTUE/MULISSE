#include "Modules/CalcFfts.hpp"
#include "Util/typedefs.hpp"
#include "Util/RunSettings.hpp"
#include "Util/Logger.hpp"

void calculate_ffts(bool normalized) {
    auto &RS = RunSettings::get_instance();
    uint series_len = RS.get_dataset_props().series_len;
    MtsNumChannelsT num_channels = RS.get_dataset_props().num_channels;

    IndexLogger::initialize({
        .index_format = ArchiveType::NONE,
        .l_min = 0,
        .l_max = 0,
        .series_len = series_len,
        .num_channels = num_channels,
        .normalized = normalized,
        .index_params = nullptr,
    });
    auto &logger = IndexLogger::get_instance();

    if (RS.ffts_supported()) {
        logger.start_timer(ISC::FFT_CALC_TIME_S);
        RS.calculate_ffts();
        logger.stop_timer(ISC::FFT_CALC_TIME_S);
    }

    logger.write_entry();
}

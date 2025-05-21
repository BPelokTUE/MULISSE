#include "Util/Stats/IndexStats.hpp"
#include "Util/Stats/IndexStatsExtractor/EnvelopeStdExtractor.hpp"

vec<Real> EnvelopeStdExtractor::extract(const IndexStats &stats) const {
    return {
        stats.m_seg_lower_stats.m_mean,   stats.m_seg_lower_stats.m_st_dev, stats.m_seg_upper_stats.m_mean,
        stats.m_seg_upper_stats.m_st_dev, stats.m_seg_mid_stats.m_mean,     stats.m_seg_mid_stats.m_st_dev,
    };
}

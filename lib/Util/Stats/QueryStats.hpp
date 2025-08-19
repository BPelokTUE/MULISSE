#ifndef UTIL_STATS_QUERYSTATS_HPP
#define UTIL_STATS_QUERYSTATS_HPP

#include "Util/Stats/AttributesStats.hpp"

struct QueryStats {
    AttributeStats m_dist_stats;
    size_t m_subs_count = 0;
    Real m_rc_using_max, m_rc_using_mean;

    QueryStats() = default;

    void calculate();
};

#endif  // UTIL_STATS_QUERYSTATS_HPP

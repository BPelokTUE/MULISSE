#include "Util/Stats/QueryStats.hpp"

#include "Util/HelperFuncs/Conversion.hpp"

void QueryStats::calculate() {
    m_dist_stats.calculate(U(m_subs_count));
    m_rc_using_max = (m_dist_stats.m_max - m_dist_stats.m_min) / m_dist_stats.m_min;
    m_rc_using_mean = m_dist_stats.m_mean / m_dist_stats.m_min;
}
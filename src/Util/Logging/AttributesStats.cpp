#include "Util/Logging/AttributesStats.hpp"

#include <algorithm>

#include "Util/Constants/Math.hpp"
#include "Util/HelperFuncs/Conversion.hpp"
#include "Util/HelperFuncs/Math.hpp"

AttributeStats::AttributeStats() {
    m_min = INF;
    m_max = m_sum = m_sum_sq = 0;
}

void AttributeStats::update(Real value, size_t count) {
    m_min = std::min(m_min, value);
    m_max = std::max(m_max, value);
    m_sum += value * R(count);
    m_sum_sq += value * value * R(count);
}

void AttributeStats::calculate(uint count) {
    if (count > 0) {
        auto mu_and_sigma = calculate_mu_and_sigma(m_sum, m_sum_sq, count);
        m_mean = mu_and_sigma.first;
        m_st_dev = mu_and_sigma.second;
    } else {
        m_min = m_max = m_mean = m_st_dev = std::numeric_limits<Real>::quiet_NaN();
    }
}

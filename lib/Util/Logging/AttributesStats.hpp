#ifndef ATTRIBUTE_STATS_HPP
#define ATTRIBUTE_STATS_HPP

#include "Util/typedefs.hpp"
#include "Util/utilities.hpp"

#define DEFINE_STAT_COLUMNS(ENUM_SUFFIX) MIN_##ENUM_SUFFIX, MAX_##ENUM_SUFFIX, MEAN_##ENUM_SUFFIX, STD_##ENUM_SUFFIX

// clang-format off
#define ADD_STATS_TO_ROW(ENUM, SUFFIX, stats)       \
    {ENUM::MIN_##SUFFIX, to_string(stats.m_min)},   \
    {ENUM::MAX_##SUFFIX, to_string(stats.m_max)},   \
    {ENUM::MEAN_##SUFFIX, to_string(stats.m_mean)}, \
    {ENUM::STD_##SUFFIX, to_string(stats.m_st_dev)}
// clang-format on

struct AttributeStats {
    Real m_min, m_max, m_mean, m_st_dev, m_sum, m_sum_sq;

    /** @brief Construct a new AttributeStats instance */
    AttributeStats();

    /**
     * @brief Update the statistics with a new value, repeated `count` times
     * @param value The value to update the statistics with
     * @param count The number of times to repeat the value
     */
    void update(Real value, size_t count = 1);

    /**
     * @brief Calculate the statistics from the accumulated values
     * @param count The number of values used to calculate the statistics
     */
    void calculate(uint count);
};

#endif  // ATTRIBUTE_STATS_HPP

#ifndef UTIL_STATS_SCOREFUNC_SCOREFUNC_HPP
#define UTIL_STATS_SCOREFUNC_SCOREFUNC_HPP

#include "Util/Types/Numbers.hpp"
#include "Util/Types/Vec.hpp"

/** @brief Class for calculating scores of real-valued inputs */
class IScoreFunc {
   public:
    virtual ~IScoreFunc() = default;

    /**
     * @brief Calculate the score based on the input
     * @param input The input to calculate the score from
     * @return The calculated score
     */
    virtual Real calculate_score(const vec<Real> &input) const = 0;
};

#endif  // UTIL_STATS_SCOREFUNC_SCOREFUNC_HPP

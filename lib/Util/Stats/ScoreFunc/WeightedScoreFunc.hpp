#ifndef UTIL_STATS_WEIGHTEDSCOREFUNC_RIDGEREGRESSION_HPP
#define UTIL_STATS_WEIGHTEDSCOREFUNC_RIDGEREGRESSION_HPP

#include "Util/Stats/ScoreFunc/ScoreFunc.hpp"

class WeightedScoreFunc : public IScoreFunc {
   public:
    WeightedScoreFunc(vec<Real> coefficients, Real intercept);

    WeightedScoreFunc(const str &weights_file);

    Real calculate_score(const vec<Real> &input) const override;

   private:
    vec<Real> m_coefficients;
    Real m_intercept;
};

#endif  // UTIL_STATS_WEIGHTEDSCOREFUNC_RIDGEREGRESSION_HPP

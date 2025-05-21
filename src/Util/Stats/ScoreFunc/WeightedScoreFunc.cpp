#include "Util/Stats/ScoreFunc/WeightedScoreFunc.hpp"

Real WeightedScoreFunc::calculate_score(const vec<Real> &input) const {
    if (input.size() != m_coefficients.size()) {
        throw std::invalid_argument("Input size does not match weights size");
    }

    Real score = m_intercept;
    for (size_t i = 0; i < input.size(); ++i) {
        score += m_coefficients[i] * input[i];
    }
    return score;
}

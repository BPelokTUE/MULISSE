#include "Util/Stats/ScoreFunc/WeightedScoreFunc.hpp"

#include <fstream>
#include <stdexcept>

WeightedScoreFunc::WeightedScoreFunc(vec<Real> coefficients, Real intercept)
    : m_coefficients(std::move(coefficients)), m_intercept(intercept) {}

WeightedScoreFunc::WeightedScoreFunc(const str &weights_file) {
    std::ifstream weights_ifs(weights_file);
    Real coef;
    while (weights_ifs >> coef) m_coefficients.push_back(coef);
    m_intercept = m_coefficients.back();
    m_coefficients.pop_back();
}

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

#include <vector>
#include <cmath>
#include <algorithm>
#include <limits>

#include "Summarization/iSaxBreakpointStrategy.hpp"

EquiprobableBreakpointStrategy::EquiprobableBreakpointStrategy(float mean, float standard_deviation)
    : m_distribution(mean, standard_deviation) {};

vec<float> EquiprobableBreakpointStrategy::get_breakpoints(SaxSymbolT alphabet_size) const {
    vec<float> thresholds(alphabet_size - 1);
    for (size_t i = 0; i < alphabet_size - 1; ++i) {
        float p = (i + 1) / static_cast<float>(alphabet_size);
        thresholds[i] = boost::math::quantile(m_distribution, p);
    }
    return thresholds;
}

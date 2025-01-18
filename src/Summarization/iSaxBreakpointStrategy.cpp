#include <vector>
#include <cmath>
#include <algorithm>
#include <limits>

#include "Summarization/iSaxBreakpointStrategy.hpp"

EquiprobableStrategy::EquiprobableStrategy(float standard_deviation) : m_distribution(0.0, standard_deviation) {};

vec<float> EquiprobableStrategy::get_breakpoints(SaxSymbolT alphabet_size) const {
    vec<float> thresholds(alphabet_size + 1);
    thresholds[0] = -std::numeric_limits<float>::infinity();
    thresholds[alphabet_size] = std::numeric_limits<float>::infinity();

    for (size_t i = 1; i < alphabet_size; ++i) {
        float p = i / static_cast<float>(alphabet_size);
        thresholds[i] = boost::math::quantile(m_distribution, p);
    }
    return thresholds;
}

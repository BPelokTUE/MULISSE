#include <vector>
#include <cmath>
#include <algorithm>
#include <limits>

#include "Summarization/iSaxBreakpointStrategy.hpp"

EquiprobableStrategy::EquiprobableStrategy(float standard_deviation) : m_distribution(0.0, standard_deviation) {};

vec<float> EquiprobableStrategy::get_breakpoints(SaxSymbolT alphabet_size) const {
    vec<float> thresholds(alphabet_size - 1);
    for (size_t i = 0; i < alphabet_size - 1; ++i) {
        float p = (i + 1) / static_cast<float>(alphabet_size);
        thresholds[i] = boost::math::quantile(m_distribution, p);
    }
    return thresholds;
}

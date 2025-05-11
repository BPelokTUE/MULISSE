
#include "Index/Sax/SaxBreakpointStrategy.hpp"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <limits>
#include <vector>

#include "Util/HelperFuncs/Conversion.hpp"

// Equiprobable breakpoint strategy

EquiprobableBreakpointStrategy::EquiprobableBreakpointStrategy(Real mean, Real standard_deviation)
    : m_distribution(mean, standard_deviation) {};

vec<Real> EquiprobableBreakpointStrategy::get_breakpoints(SaxSymbolT alphabet_size) const {
    vec<Real> thresholds(alphabet_size - 1);
    for (SaxSymbolT i = 0; i < alphabet_size - 1; ++i) {
        Real p = R(i + 1) / R(alphabet_size);
        thresholds[i] = boost::math::quantile(m_distribution, p);
    }
    return thresholds;
}

void EquiprobableBreakpointStrategy::adapt_to_dataset(Real mu, Real sigma) {
    m_distribution = boost::math::normal_distribution<Real>(mu, sigma);
}

// Fixed breakpoint strategy

FixedBreakpointStrategy::FixedBreakpointStrategy(const str &file) {
    std::ifstream ifs(file);
    if (!ifs.is_open()) {
        throw std::runtime_error("Could not open file: " + file);
    }

    Real breakpoint;
    while (ifs >> breakpoint) {
        m_breakpoints.push_back(breakpoint);
    }
}

vec<Real> FixedBreakpointStrategy::get_breakpoints(SaxSymbolT alphabet_size) const {
    if (alphabet_size > m_breakpoints.size() + 1) {
        throw std::runtime_error("Requested alphabet size exceeds the number of breakpoints available.");
    }
    if (alphabet_size == m_breakpoints.size() + 1) {
        return m_breakpoints;
    }

    vec<Real> breakpoints(alphabet_size - 1);
    SaxSymbolT alphabet_ratio = static_cast<SaxSymbolT>((m_breakpoints.size() + 1) / alphabet_size);
    for (SaxSymbolT i = 1; i < alphabet_size; ++i) {
        breakpoints[i] = m_breakpoints[static_cast<SaxSymbolT>(i * alphabet_ratio - 1)];
    }
    return breakpoints;
}

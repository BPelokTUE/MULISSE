#include "Index/Sax/BreakpointStrategy/EquiprobableBreakpointStrategy.hpp"

#include "Util/HelperFuncs/Conversion.hpp"

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

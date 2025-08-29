#include "Index/Sax/BreakpointStrategy/EquiprobableBreakpointStrategy.hpp"

#include "Index/Sax/SaxBreakpoints.hpp"
#include "Util/HelperFuncs/Conversion.hpp"

EquiprobableBreakpointStrategy::EquiprobableBreakpointStrategy(Real mean, Real standard_deviation)
    : m_distribution(mean, standard_deviation) {};

SaxBreakpoints EquiprobableBreakpointStrategy::get_breakpoints(SaxSymbolT alphabet_size) const {
    SaxBreakpoints breakpoints(alphabet_size - 1);
    for (SaxSymbolT i = 0; i < alphabet_size - 1; ++i) {
        Real p = R(i + 1) / R(alphabet_size);
        breakpoints[i] = boost::math::quantile(m_distribution, p);
    }
    return breakpoints;
}

void EquiprobableBreakpointStrategy::adapt_to_dataset(Real mu, Real sigma) {
    m_distribution = boost::math::normal_distribution<Real>(mu, sigma);
}

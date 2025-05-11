#include "Index/Entry/Paa.hpp"

#include "Util/HelperFuncs/Conversion.hpp"
#include "Util/HelperFuncs/Math.hpp"

vec<Real> paa(const vec<Real> &ts, const ISegmentationStrategy *segmentation_strategy) {
    uint num_segments = segmentation_strategy->get_num_segments(U(ts.size()));
    vec<Real> paa_values(num_segments);

    Real sum;
    uint ts_ind = 0;
    for (SaxSegIndT s = 0; s < num_segments; ++s) {
        sum = 0;
        uint segment_len = segmentation_strategy->get_segment_len(s);
        for (uint i = 0; i < segment_len; ++i, ++ts_ind) sum += ts[ts_ind];
        paa_values[s] = sum / R(segment_len);
    }
    return paa_values;
}

Paa::Paa(const vec<Real> &paa_values) : m_paa_values(paa_values) {}

bool Paa::operator==(const Paa &other) const { return m_paa_values == other.m_paa_values; }

size_t Paa::size() const { return m_paa_values.size(); }

void Paa::resize(size_t new_size) { m_paa_values.resize(new_size); }

const vec<Real> &Paa::get_isax_input() const { return m_paa_values; }

#include "Util/Types/LengthRange.hpp"

#include <cereal/archives/json.hpp>

void LengthRange::validate() const {
    if (m_l_min > m_l_max) {
        throw std::invalid_argument(
            std::format("Minimum length is greater than maximum length: {} > {}", m_l_min, m_l_max));
    }
}

template <typename Archive>
void LengthRange::serialize(Archive &ar) {
    ar(cereal::make_nvp("l_min", m_l_min), cereal::make_nvp("l_max", m_l_max));
}

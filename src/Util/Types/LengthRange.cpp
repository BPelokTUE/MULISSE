#include "Util/Types/LengthRange.hpp"

#include <cereal/archives/json.hpp>

template <typename Archive>
void LengthRange::serialize(Archive &ar) {
    ar(cereal::make_nvp("l_min", m_l_min), cereal::make_nvp("l_max", m_l_max));
}

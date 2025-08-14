#include "Util/RunSettings/LengthProperties.hpp"

#include <cereal/archives/json.hpp>
#include <fstream>

#include "Util/Types/String.hpp"

void LengthProperties::set_lengths_per_group(uint l_per_group) {
    m_l_per_group = l_per_group;
    if (m_l_per_group > 0) {
        m_num_l_groups = static_cast<uint>((m_l_max - m_l_min + m_l_per_group) / m_l_per_group);
        m_use_length_groups = true;
    }
}

void LengthProperties::save(const str &out_path) const {
    std::ofstream ofs(out_path);
    cereal::JSONOutputArchive ar(ofs);
    ar(cereal::make_nvp("use_length_groups", m_use_length_groups), cereal::make_nvp("l_min", m_l_min),
       cereal::make_nvp("l_max", m_l_max), cereal::make_nvp("l_per_group", m_l_per_group),
       cereal::make_nvp("num_l_groups", m_num_l_groups));
}

void LengthProperties::load(const str &in_path) {
    std::ifstream ifs(in_path);
    if (!ifs.is_open()) throw std::runtime_error("Could not open length properties file: " + in_path);
    cereal::JSONInputArchive ar(ifs);
    ar(cereal::make_nvp("use_length_groups", m_use_length_groups), cereal::make_nvp("l_min", m_l_min),
       cereal::make_nvp("l_max", m_l_max), cereal::make_nvp("l_per_group", m_l_per_group),
       cereal::make_nvp("num_l_groups", m_num_l_groups));
}

const str LengthProperties::DEFAULT_FILE_NAME = "length_properties.json";

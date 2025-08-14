#include "Util/Artefacts/MtsQueryset.hpp"

#include <cereal/archives/json.hpp>

#include "Util/HelperFuncs/Conversion.hpp"
#include "Util/HelperFuncs/Math.hpp"
#include "Util/Types/MtsQuery.hpp"

MtsQueryset::MtsQueryset() = default;

MtsQueryset::MtsQueryset(const MtsQuerysetSettings &queryset_settings) : m_settings(queryset_settings) {}

template <typename Archive>
void MtsQueryset::apply_archive(Archive &ar) {
    ar(cereal::make_nvp("num_queries", m_settings.m_num_queries),
       cereal::make_nvp("length_range", m_settings.m_length_range));
}

void MtsQueryset::save(const str &out_file, ArchiveType) {
    std::ofstream ofs(out_file);
    if (!ofs.is_open()) throw std::runtime_error("Failed to open output file: " + out_file);
    cereal::JSONOutputArchive ar(ofs);

    apply_archive(ar);
}

void MtsQueryset::load(const str &in_file, ArchiveType) {
    m_queryset_ifs.open(in_file);
    if (!m_queryset_ifs.is_open()) throw std::runtime_error("Failed to open input file: " + in_file);
    cereal::JSONInputArchive ar(m_queryset_ifs);

    apply_archive(ar);
}

const MtsQuerysetSettings &MtsQueryset::get_settings() const { return m_settings; }

str MtsQueryset::get_meta_path() const {
    str path = m_settings.m_queryset_path;
    path.replace(path.find_last_of('.'), path.size() - path.find_last_of('.'), "_qs_meta.json");
    return path;
}

MtsQuery MtsQueryset::load_next_query(bool normalized) {
    if (!m_queryset_ifs.is_open()) throw std::runtime_error("Queryset file is not open: " + m_settings.m_queryset_path);

    vec<vec<Real>> query_data;

    for (MtsNumChannelsT c = 0; !m_queryset_ifs.eof();) {
        str line;
        std::getline(m_queryset_ifs, line);
        std::istringstream iss(line);

        query_data[c].clear();
        Real value;

        if (normalized) {
            Real sum = 0, sq_sum = 0;
            query_data[c].clear();
            while (iss >> value) {
                query_data[c].push_back(value);
                sum += value;
                sq_sum += value * value;
            }
            if (!query_data[c].empty()) {
                auto [mu, sigma] = calculate_mu_and_sigma(sum, sq_sum, U(query_data[c].size()));
                for (size_t i = 0; i < query_data[c].size(); ++i) query_data[c][i] = (query_data[c][i] - mu) / sigma;
            }
        } else {
            while (iss >> value) query_data[c].push_back(value);
        }
    }

    return MtsQuery(std::move(query_data));
}

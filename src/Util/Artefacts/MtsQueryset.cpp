#include "Util/Artefacts/MtsQuerySet.hpp"

#include <cereal/archives/json.hpp>

#include "Util/Artefacts/MtsDataset.hpp"
#include "Util/HelperFuncs/Conversion.hpp"
#include "Util/HelperFuncs/Math.hpp"
#include "Util/Types/MtsQuery.hpp"

MtsQuerySet::MtsQuerySet() = default;

MtsQuerySet::MtsQuerySet(const MtsDataset *dataset, const MtsQuerySetProperties &query_set_props,
                         uptr<std::ostream> ostream)
    : m_properties(query_set_props) {
    set_source_dataset(dataset);
    set_ostream(std::move(ostream));
}

template <typename Archive>
void MtsQuerySet::apply_archive(Archive &ar) {
    ar(cereal::make_nvp("num_queries", m_properties.m_num_queries),
       cereal::make_nvp("length_range", m_properties.m_length_range));
}

void MtsQuerySet::save(const str &out_file, ArchiveType) {
    std::ofstream ofs(out_file);
    if (!ofs.is_open()) throw std::runtime_error("Failed to open output file: " + out_file);
    cereal::JSONOutputArchive ar(ofs);

    apply_archive(ar);
}

void MtsQuerySet::load(const str &in_file, ArchiveType) {
    std::ifstream ifs(in_file);
    if (!ifs.is_open()) throw std::runtime_error("Failed to open input file: " + in_file);
    cereal::JSONInputArchive ar(ifs);

    apply_archive(ar);
}

const MtsQuerySetProperties &MtsQuerySet::get_properties() const { return m_properties; }

str MtsQuerySet::get_meta_path() const {
    str path = m_properties.m_query_set_path;
    path.replace(path.find_last_of('.'), path.size() - path.find_last_of('.'), "_qs_meta.json");
    return path;
}

void MtsQuerySet::set_source_dataset(const MtsDataset *source_dataset) {
    if (!source_dataset) {
        throw std::runtime_error("Source dataset cannot be null.");
    }
    m_source_dataset = source_dataset;
}

const MtsDataset *MtsQuerySet::get_source_dataset() const {
    if (!m_source_dataset) {
        throw std::runtime_error("Source dataset is not set.");
    }
    return m_source_dataset;
}

void MtsQuerySet::set_istream(uptr<std::istream> istream) {
    if (!istream || !(*istream) || !istream->good()) {
        throw std::runtime_error("Failed to set input stream for MtsQuerySet: stream is not valid.");
    }
    m_istream = std::move(istream);
}

void MtsQuerySet::set_ostream(uptr<std::ostream> ostream) {
    if (!ostream || !(*ostream) || !ostream->good()) {
        throw std::runtime_error("Failed to set output stream for MtsQuerySet: stream is not valid.");
    }
    m_ostream = std::move(ostream);
}

MtsQuery MtsQuerySet::load_next_query(bool normalized) {
    MtsNumChannelsT num_channels = m_source_dataset->get_properties().m_num_channels;
    vec<vec<Real>> query_data(num_channels);

    for (MtsNumChannelsT c = 0; !m_istream->eof() && c < num_channels; ++c) {
        str line;
        // m_istream->getline(line);
        std::getline(*m_istream, line);
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

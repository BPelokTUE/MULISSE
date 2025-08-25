#include "Util/Artefacts/MtsQuerySet.hpp"

#include <algorithm>
#include <cereal/archives/json.hpp>

#include "Util/Artefacts/MtsDataset.hpp"
#include "Util/Artefacts/Options/QuerySetGenOptions.hpp"
#include "Util/HelperFuncs/Conversion.hpp"
#include "Util/HelperFuncs/Math.hpp"
#include "Util/HelperFuncs/Path.hpp"
#include "Util/Types/MtsQuery.hpp"
#include "Util/Types/SubsequenceInfo.hpp"

MtsQuerySet::MtsQuerySet() = default;

MtsQuerySet::MtsQuerySet(MtsDataset &dataset, const MtsQuerySetProperties &query_set_props, OutputStream &&ostream)
    : m_properties(query_set_props) {
    set_source_dataset(dataset);
    m_ostream = std::move(m_ostream);
}

template <typename Archive>
void MtsQuerySet::apply_archive(Archive &ar) {
    ar(cereal::make_nvp("log_id", m_log_id), cereal::make_nvp("num_queries", m_properties.m_num_queries),
       cereal::make_nvp("length_range", m_properties.m_length_range),
       cereal::make_nvp("source_dataset_path", m_properties.m_source_dataset_path),
       cereal::make_nvp("query_set_file", m_properties.m_query_set_path));
}

void MtsQuerySet::apply_in_archive(cereal::JSONInputArchive &ar) { apply_archive(ar); }

void MtsQuerySet::apply_out_archive(cereal::JSONOutputArchive &ar) { apply_archive(ar); }

const MtsQuerySetProperties &MtsQuerySet::get_properties() const { return m_properties; }

str MtsQuerySet::get_meta_path() const { return append_to_base(m_properties.m_query_set_path, "_qs_meta"); }

void MtsQuerySet::set_source_dataset(MtsDataset &source_dataset) { m_source_dataset = source_dataset; }

MtsDataset &MtsQuerySet::get_source_dataset() const {
    if (!m_source_dataset) {
        throw std::runtime_error("Source dataset is not set.");
    }
    return m_source_dataset->get();
}

MtsQuery MtsQuerySet::load_next_query(bool normalized) {
    MtsNumChannelsT num_channels = m_source_dataset->get().get_properties().m_num_channels;
    vec<vec<Real>> query_data(num_channels);

    for (MtsNumChannelsT c = 0; !m_istream.get().eof() && c < num_channels; ++c) {
        str line;

        std::getline(m_istream.get(), line);
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

    return MtsQuery(std::move(query_data), normalized);
}

struct QueryDescriptor {
    SubsequenceInfo subs_info;
    uint length;
    vec<bool> channels;

    bool operator<(const QueryDescriptor &other) const { return subs_info < other.subs_info; }
};

void MtsQuerySet::generate(const QuerySetGenOptions &query_set_gen_opts, const vec<uint> &series_inds = {}) {
    auto [used_channels, seed, noise, exact_lengths, channel_mask] = query_set_gen_opts;
    auto [num_channels, series_len, num_series, dataset_file] = m_source_dataset->get().get_properties();
    uint num_queries = m_properties.m_num_queries;
    auto [l_min, l_max] = m_properties.m_length_range;

    bool random_lengths = l_min > 0 && l_max >= l_min;
    uint total_num_queries = random_lengths ? num_queries : num_queries * U(exact_lengths.size());
    uint num_series_inds = series_inds.empty() ? num_series : U(series_inds.size());

    vec<QueryDescriptor> query_descriptors(total_num_queries);

    std::default_random_engine rng(query_set_gen_opts.m_seed);
    std::normal_distribution<Real> noise_normal_dist(0.0, query_set_gen_opts.m_noise);
    std::uniform_int_distribution<uint> series_uniform_dist(0, num_series_inds - 1),
        channel_uniform_dist(1, num_channels), length_uniform_dist(l_min, l_max);

    auto generate_query_descriptor = [&](uint length) -> QueryDescriptor {
        uint included_channels =
            query_set_gen_opts.m_used_channels == 0 ? channel_uniform_dist(rng) : query_set_gen_opts.m_used_channels;
        vec<bool> channels(num_channels, false);
        if (query_set_gen_opts.m_channel_mask.empty()) {
            std::fill(channels.begin(), channels.begin() + included_channels, true);
            std::shuffle(channels.begin(), channels.end(), rng);
        } else {
            channels = query_set_gen_opts.m_channel_mask;
        }

        auto start_pos_dist = std::uniform_int_distribution<uint>(0, series_len - length);
        uint series_ind = series_inds.empty() ? series_uniform_dist(rng) : series_inds[series_uniform_dist(rng)];
        SubsequenceInfo subs_info = {series_ind, start_pos_dist(rng)};
        return {subs_info, length, channels};
    };

    for (uint i = 0; i < num_queries; ++i) {
        if (random_lengths) {
            query_descriptors[i] = generate_query_descriptor(length_uniform_dist(rng));
        } else {
            for (uint j = 0; j < query_set_gen_opts.m_exact_lengths.size(); ++j) {
                query_descriptors[i * query_set_gen_opts.m_exact_lengths.size() + j] =
                    generate_query_descriptor(query_set_gen_opts.m_exact_lengths[j]);
            }
        }
    }
    std::sort(query_descriptors.begin(), query_descriptors.end());

    for (size_t q = 0; q < query_descriptors.size(); ++q) {
        const auto [subs_info, length, channel_mask] = query_descriptors[q];

        auto mts = m_source_dataset->get().load_series(subs_info.m_position.m_series, channel_mask);

        for (MtsNumChannelsT c = 0; c < num_channels; ++c) {
            if (channel_mask[c]) {
                Real sum = 0, sum_sq = 0;
                for (Real value : mts[c]) {
                    sum += value;
                    sum_sq += value * value;
                }
                Real sigma = calculate_mu_and_sigma(sum, sum_sq, series_len).second;

                for (uint j = 0; j < length; ++j) {
                    Real value = mts[c][j] + noise_normal_dist(rng) * sigma;
                    m_ostream.get().write(reinterpret_cast<const char *>(&value), sizeof(value));
                    if (j < length - 1) m_ostream.get().put(' ');
                }
            }
            if (q < query_descriptors.size() - 1 || c < num_channels - 1) m_ostream.get().put('\n');
        }
    }
}

#include "Modules/QueryGen.hpp"

#include <algorithm>
#include <filesystem>
#include <random>

#include "Util/HelperFuncs/Conversion.hpp"
#include "Util/HelperFuncs/Math.hpp"
#include "Util/HelperFuncs/Path.hpp"
#include "Util/Logging/QuerySetLogger.hpp"
#include "Util/Types/RunContext.hpp"
#include "Util/Types/SubsequenceInfo.hpp"

struct QueryDescriptor {
    SubsequenceInfo subs_info;
    uint length;
    vec<bool> channels;

    bool operator<(const QueryDescriptor &other) const { return subs_info < other.subs_info; }
};

bool get_use_random_lengths(const LengthRange &length_range) {
    return length_range.m_l_min > 0 && length_range.m_l_max >= length_range.m_l_min;
}

uint get_total_num_queries(bool random_lengths, uint num_queries, const vec<uint> &exact_lengths) {
    return random_lengths ? num_queries : num_queries * U(exact_lengths.size());
}

void create_queries(MtsQuerySet &query_set, const QuerySetGenOptions &query_set_gen_opts,
                    const RunContext &run_context) {
    auto [normalized, seed, data_path, logs_path] = run_context;

    auto dataset_props = query_set.get_source_dataset()->get_properties();
    auto query_set_props = query_set.get_properties();

    MtsNumChannelsT num_channels = dataset_props.m_num_channels;
    str dataset_path = std::filesystem::path(data_path) / dataset_props.m_dataset_path;
    str query_path = std::filesystem::path(data_path) / query_set_props.m_query_set_path;

    // Extract time series from dataset
    std::ifstream data_file(dataset_path, std::ios::binary);
    std::ofstream query_file(query_path);

    generate_queries(data_file, query_file, dataset_props, query_set_props, query_set_gen_opts);

    auto query_set_props_to_log = query_set_props;
    auto query_gen_opts_to_log = query_set_gen_opts;

    if (get_use_random_lengths(query_set_props.m_length_range)) {
        query_gen_opts_to_log.m_exact_lengths = vec<uint>{};
    } else {
        query_set_props_to_log.m_length_range.m_l_min = 0;
        query_set_props_to_log.m_length_range.m_l_max = 0;
    }
    query_gen_opts_to_log.m_used_channels =
        query_set_gen_opts.m_channel_mask.empty() ? query_set_gen_opts.m_used_channels : 0;

    QuerySetLogger::write_entry(dataset_props, query_set_props_to_log, query_gen_opts_to_log, logs_path);
}

void generate_queries(std::istream &data_is, std::ostream &query_os, const MtsDatasetProperties &dataset_props,
                      const MtsQuerySetProperties &query_set_props, const QuerySetGenOptions &query_set_gen_opts,
                      const vec<uint> &series_inds) {
    bool random_lengths = get_use_random_lengths(query_set_props.m_length_range);
    uint total_num_queries =
        get_total_num_queries(random_lengths, query_set_props.m_num_queries, query_set_gen_opts.m_exact_lengths);

    vec<QueryDescriptor> query_descriptors(total_num_queries);

    auto [num_channels, series_len, num_series, dataset_file] = dataset_props;
    uint num_series_inds = series_inds.empty() ? num_series : U(series_inds.size());

    std::default_random_engine rng(query_set_gen_opts.m_seed);
    std::normal_distribution<Real> noise_normal_dist(0.0, query_set_gen_opts.m_noise);
    std::uniform_int_distribution<uint> series_uniform_dist(0, num_series_inds - 1),
        channel_uniform_dist(1, num_channels),
        length_uniform_dist(query_set_props.m_length_range.m_l_min, query_set_props.m_length_range.m_l_max);

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

    for (uint i = 0; i < query_set_props.m_num_queries; ++i) {
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
        const auto [subs_info, length, channels] = query_descriptors[q];
        SubsequencePosition series_pos = {.m_series = subs_info.m_position.m_series, .m_start = 0};

        for (MtsNumChannelsT c = 0; c < num_channels; ++c) {
            if (channels[c]) {
                data_is.seekg(series_pos.get_file_pos(series_len, num_channels, c));
                Real sum = 0, sum_sq = 0, value;
                for (uint j = 0; j < series_len; ++j) {
                    data_is.read(reinterpret_cast<char *>(&value), sizeof(value));
                    sum += value;
                    sum_sq += value * value;
                }
                Real sigma = calculate_mu_and_sigma(sum, sum_sq, series_len).second;

                data_is.seekg(subs_info.m_position.get_file_pos(series_len, num_channels, c));
                for (uint j = 0; j < length; ++j) {
                    data_is.read(reinterpret_cast<char *>(&value), sizeof(value));
                    value += noise_normal_dist(rng) * sigma;
                    query_os << value;
                    if (j < length - 1) query_os << ' ';
                }
            }
            if (q < query_descriptors.size() - 1 || c < num_channels - 1) {
                query_os << '\n';
            }
        }
    }
}

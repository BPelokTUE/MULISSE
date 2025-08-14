#include "Modules/QueryGen.hpp"

#include <algorithm>
#include <filesystem>
#include <random>

#include "Util/HelperFuncs/Conversion.hpp"
#include "Util/HelperFuncs/Math.hpp"
#include "Util/HelperFuncs/Path.hpp"
#include "Util/Logging/QuerySetLogger.hpp"
#include "Util/Types/SubsequenceInfo.hpp"

struct QueryDescriptor {
    SubsequenceInfo subs_info;
    uint length;
    vec<bool> channels;

    bool operator<(const QueryDescriptor &other) const { return subs_info < other.subs_info; }
};

bool get_use_random_lengths(const QuerysetGenOptions &opts) {
    return opts.m_length_range.m_l_min > 0 && opts.m_length_range.m_l_max >= opts.m_length_range.m_l_min;
}

uint get_total_num_queries(bool random_lengths, const QuerysetGenOptions &opts) {
    return random_lengths ? opts.m_num_queries : opts.m_num_queries * U(opts.m_exact_lengths.size());
}

void create_queries(MtsDataset &dataset, MtsQueryset &queryset, QuerysetGenOptions opts) {
    auto dataset_settings = dataset.get_settings();
    auto queryset_settings = queryset.get_settings();

    MtsNumChannelsT num_channels = dataset_settings.m_num_channels;
    str dataset_path = dataset_settings.m_dataset_path;
    str query_path = queryset_settings.m_queryset_path;

    // Extract time series from dataset
    std::ifstream data_file(dataset_path, std::ios::binary);
    std::ofstream query_file(query_path);

    bool random_lengths = get_use_random_lengths(opts);
    uint total_num_queries = get_total_num_queries(random_lengths, opts);

    generate_queries(data_file, query_file, opts);

    auto queryset_settings_to_log = queryset_settings;
    auto query_gen_opts_to_log = opts;

    if (random_lengths) {
        query_gen_opts_to_log.m_exact_lengths = vec<uint>{};
    } else {
        queryset_settings_to_log.m_length_range.m_l_min = 0;
        queryset_settings_to_log.m_length_range.m_l_max = 0;
    }
    query_gen_opts_to_log.m_used_channels = opts.m_channel_mask.empty() ? opts.m_used_channels : 0;
    QuerySetLogger::write_entry(queryset_settings_to_log, query_gen_opts_to_log);
}

void generate_queries(std::istream &data_is, std::ostream &query_os, const MtsDatasetSettings &dataset_settings,
                      const MtsQuerysetSettings &queryset_settings, const QuerysetGenOptions &opts,
                      const vec<uint> &series_inds) {
    bool random_lengths = get_use_random_lengths(opts);
    uint total_num_queries = get_total_num_queries(random_lengths, opts);

    vec<QueryDescriptor> query_descriptors(total_num_queries);

    auto [num_channels, series_len, num_series, dataset_file] = dataset_settings;
    uint num_series_inds = series_inds.empty() ? num_series : U(series_inds.size());

    std::default_random_engine rng(opts.m_seed);
    std::normal_distribution<Real> noise_normal_dist(0.0, opts.m_noise);
    std::uniform_int_distribution<uint> series_uniform_dist(0, num_series_inds - 1),
        channel_uniform_dist(1, num_channels),
        length_uniform_dist(opts.m_length_range.m_l_min, opts.m_length_range.m_l_max);

    auto generate_query_descriptor = [&](uint length) -> QueryDescriptor {
        uint included_channels = opts.m_used_channels == 0 ? channel_uniform_dist(rng) : opts.m_used_channels;
        vec<bool> channels(num_channels, false);
        if (opts.m_channel_mask.empty()) {
            std::fill(channels.begin(), channels.begin() + included_channels, true);
            std::shuffle(channels.begin(), channels.end(), rng);
        } else {
            channels = opts.m_channel_mask;
        }

        auto start_pos_dist = std::uniform_int_distribution<uint>(0, series_len - length);
        uint series_ind = series_inds.empty() ? series_uniform_dist(rng) : series_inds[series_uniform_dist(rng)];
        SubsequenceInfo subs_info = {series_ind, start_pos_dist(rng)};
        return {subs_info, length, channels};
    };

    for (uint i = 0; i < opts.m_num_queries; ++i) {
        if (random_lengths) {
            query_descriptors[i] = generate_query_descriptor(length_uniform_dist(rng));
        } else {
            for (uint j = 0; j < opts.m_exact_lengths.size(); ++j) {
                query_descriptors[i * opts.m_exact_lengths.size() + j] =
                    generate_query_descriptor(opts.m_exact_lengths[j]);
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

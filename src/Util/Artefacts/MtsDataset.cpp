#include "Util/Artefacts/MtsDataset.hpp"

#include <algorithm>
#include <cereal/archives/json.hpp>
#include <fstream>
#include <string>

#include "Util/Artefacts/Options/CsvDatasetGenOptions.hpp"
#include "Util/Artefacts/Options/RandomWalkGenOptions.hpp"
#include "Util/HelperFuncs/Math.hpp"
#include "Util/Types/MultivariateTimeSeries.hpp"
#include "Util/Types/Numbers.hpp"
#include "Util/Types/Vec.hpp"

MtsDataset::MtsDataset() = default;

MtsDataset::MtsDataset(const MtsDatasetProperties &dataset_props, const str &data_path) : m_properties(dataset_props) {
    str dataset_path = std::filesystem::path(data_path) / m_properties.m_dataset_path;
    set_ostream(std::make_unique<std::ofstream>(dataset_path, std::ios::binary));
    std::filesystem::create_directories(std::filesystem::path(dataset_path).parent_path());
}

template <typename Archive>
void MtsDataset::apply_archive(Archive &ar) {
    ar(cereal::make_nvp("log_id", m_log_id), cereal::make_nvp("num_channels", m_properties.m_num_channels),
       cereal::make_nvp("series_len", m_properties.m_series_len),
       cereal::make_nvp("num_series", m_properties.m_num_series),
       cereal::make_nvp("dataset_file", m_properties.m_dataset_path),
       cereal::make_nvp("channel_stats", m_channel_stats));
}

void MtsDataset::save(const str &out_file, ArchiveType) {
    std::ofstream ofs(out_file);
    cereal::JSONOutputArchive ar(ofs);

    apply_archive(ar);
}

void MtsDataset::load(const str &in_file, ArchiveType) {
    std::ifstream ifs(in_file);
    if (!ifs.is_open()) throw std::runtime_error("Could not open dataset meta file: " + in_file);
    cereal::JSONInputArchive ar(ifs);

    apply_archive(ar);
}

const MtsDatasetProperties &MtsDataset::get_properties() const { return m_properties; }

str MtsDataset::get_meta_path() const {
    str path = m_properties.m_dataset_path;
    path.replace(path.find_last_of('.'), path.size() - path.find_last_of('.'), "_ds_meta.json");
    return path;
}

size_t MtsDataset::get_size_on_disk() const {
    return static_cast<size_t>(m_properties.m_num_channels) * m_properties.m_series_len * sizeof(Real) *
           m_properties.m_num_series;
}

void MtsDataset::set_ostream(uptr<std::ostream> ostream) {
    if (!ostream || !(*ostream) || !ostream->good()) {
        throw std::runtime_error("Failed to set output stream for MtsDataset: stream is not valid.");
    }
    m_ostream = std::move(ostream);
}

void MtsDataset::set_istream(uptr<std::istream> istream) {
    if (!istream || !(*istream) || !istream->good()) {
        throw std::runtime_error("Failed to set input stream for MtsDataset: stream is not valid.");
    }
    m_istream = std::move(istream);
}

void MtsDataset::generate_random_walks(const RandomWalkGenOptions &rw_gen_opts) {
    auto [num_channels, series_len, num_series, dataset_file] = m_properties;

    std::default_random_engine generator(rw_gen_opts.m_seed);
    std::normal_distribution<Real> distribution(0.0, rw_gen_opts.m_step_sd);

    vec<Real> sums(num_channels, R(0.0)), sum_sqs(num_channels, R(0.0));

    for (uint i = 0; i < num_series; ++i) {
        for (uint j = 0; j < num_channels; ++j) {
            Real value = rw_gen_opts.m_zero_start ? 0 : distribution(generator);
            for (uint k = 0; k < series_len; ++k) {
                value += distribution(generator);
                sums[j] += value;
                sum_sqs[j] += value * value;
                // Cast the reference to `value` into `const char` pointer, so `outfile.write` will
                // try to write the bytes stored in `value` as chars. By definition a `char` contains
                // a single byte, so `sizeof(value)` can be used to specify how many chars to write.
                m_ostream->write(reinterpret_cast<const char *>(&value), sizeof(value));
            }
        }
    }
    m_channel_stats = ChannelStats(sums, sum_sqs, series_len, num_series);
}

void MtsDataset::generate_from_csvs(const CsvDatasetGenOptions &csv_gen_opts) {
    auto [num_channels, series_len, num_series, dataset_file] = m_properties;
    auto [col_sep, seed, min_subs_sd, l_range, source_csvs] = csv_gen_opts;
    auto [l_min, l_max] = l_range;

    str line;
    vec<vec<Real>> mts_data(num_channels, vec<Real>(series_len));
    vec<vec<vec<Real>>> all_mts_data;
    MtsNumChannelsT channel_ind = 0;
    bool discard = false;

    vec<std::ifstream> csv_streams;
    for (str csv_path : source_csvs) {
        csv_streams.emplace_back(csv_path);
    }

    while (true) {
        std::getline(csv_streams[channel_ind], line);

        if (!discard) {
            std::istringstream iss(line);
            str value;
            Real sum = 0, sum_sq = 0;
            uint ind = 0;

            while (std::getline(iss, value, col_sep)) {
                // Discard series if a values is not a valid number
                try {
                    mts_data[channel_ind][ind] = R(std::stod(value));
                } catch (const std::exception &e) {
                    discard = true;
                    goto next_channel;
                }

                ++ind;
                // If required, check whether the standard deviation of all relevant-length subsequences is above the
                // provided threshold. If that is not the case, discard the series.
                if (min_subs_sd > 0) {
                    sum += mts_data[channel_ind][ind - 1];
                    sum_sq += mts_data[channel_ind][ind - 1] * mts_data[channel_ind][ind - 1];

                    uint start_min = U(std::max(0, static_cast<int>(ind - l_max)));
                    int start_max = static_cast<int>(ind - l_min);
                    Real sum_tmp = sum, sum_sq_tmp = sum_sq;
                    for (uint start = start_min; static_cast<int>(start) <= start_max; ++start) {
                        Real sigma = calculate_mu_and_sigma(sum_tmp, sum_sq_tmp, U(ind - start)).second;
                        if (sigma < min_subs_sd) {
                            discard = true;
                            goto next_channel;
                        }
                        sum_tmp -= mts_data[channel_ind][start];
                        sum_sq_tmp -= mts_data[channel_ind][start] * mts_data[channel_ind][start];
                    }
                    if (ind >= l_max) {
                        sum -= mts_data[channel_ind][start_min];
                        sum_sq -= mts_data[channel_ind][start_min] * mts_data[channel_ind][start_min];
                    }
                }
                if (ind == series_len) break;
            }
            // Discard series if it does not have enough values
            if (ind < series_len) discard = true;
        }
    next_channel:
        if (++channel_ind == num_channels) {
            if (!discard) all_mts_data.push_back(mts_data);
            channel_ind = 0;
            discard = false;
        }
        if (csv_streams[channel_ind].eof()) break;
    }

    if (all_mts_data.empty()) {
        throw std::runtime_error("Error: No valid time series found in the dataset");
    }

    // Select a subset of all non-discarded time series
    std::default_random_engine generator(seed);

    vec<uint> mts_indexes(all_mts_data.size());
    std::iota(mts_indexes.begin(), mts_indexes.end(), 0);
    std::shuffle(mts_indexes.begin(), mts_indexes.end(), generator);
    if (mts_indexes.size() > num_series) mts_indexes.resize(num_series);

    vec<Real> sums(num_channels, R(0.0)), sum_sqs(num_channels, R(0.0));
    for (uint mts_ind : mts_indexes) {
        for (MtsNumChannelsT c = 0; c < num_channels; ++c) {
            auto &ts = all_mts_data[mts_ind][c];
            m_ostream->write(reinterpret_cast<const char *>(ts.data()), sizeof(Real) * series_len);
            for (uint i = 0; i < series_len; ++i) {
                sums[c] += ts[i];
                sum_sqs[c] += ts[i] * ts[i];
            }
        }
    }

    m_channel_stats = ChannelStats(sums, sum_sqs, series_len, U(mts_indexes.size()));
}

MultivariateTimeSeries MtsDataset::load_series(uint series_index, const vec<bool> &channel_mask) {
    m_istream->seekg(series_index * m_properties.m_num_channels * m_properties.m_series_len * sizeof(Real));
    load_next_series(channel_mask);
}

MultivariateTimeSeries MtsDataset::load_next_series(const vec<bool> &channel_mask) {
    vec<vec<Real>> mts_data(m_properties.m_num_channels, vec<Real>(m_properties.m_series_len));
    for (MtsNumChannelsT c = 0; c < m_properties.m_num_channels; ++c) {
        size_t channel_size = m_properties.m_series_len * sizeof(Real);
        if (!channel_mask.empty() && !channel_mask[c]) {
            m_istream->ignore(channel_size);
        } else {
            m_istream->read(reinterpret_cast<char *>(mts_data[c].data()), channel_size);
        }
    }
    return MultivariateTimeSeries(std::move(mts_data));
}

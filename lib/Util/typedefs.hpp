#ifndef TYPEDEFS_HPP
#define TYPEDEFS_HPP

#include <cassert>
#include <cstdint>
#include <fstream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <cereal/access.hpp>

template <typename T>
using vec = std::vector<T>;

template <typename K, typename V>
using umap = std::unordered_map<K, V>;

template <typename K, typename V, typename H>
using umap_hash = std::unordered_map<K, V, H>;

template <typename T>
using uptr = std::unique_ptr<T>;

template <typename T>
using sptr = std::shared_ptr<T>;

using str = std::string;

using uint = uint32_t;
using SaxNumBitsT = uint8_t;
using SaxSegIndT = uint16_t;
using SaxSymbolT = uint16_t;
using MtsNumChannelsT = uint16_t;

struct SaxSplitIndex {
    SaxSegIndT seg_ind;
    MtsNumChannelsT channel;

    bool operator==(const SaxSplitIndex &other) const { return seg_ind == other.seg_ind && channel == other.channel; }

    // Required for Cereal (de)serialization
    friend class cereal::access;

    template <class Archive>
    void serialize(Archive &ar) {
        ar(seg_ind, channel);
    }
};

struct SubsequenceInfo {
    /** @brief Index of the series within the file */
    uint series_ind;
    /** @brief Index of the start position of the subsequence within the series */
    uint start_pos;
    /** @brief Length of the subsequence */
    uint length;

    bool operator<(const SubsequenceInfo &other) const {
        return series_ind < other.series_ind || (series_ind == other.series_ind && start_pos < other.start_pos);
    }

    bool operator==(const SubsequenceInfo &other) const {
        return series_ind == other.series_ind && start_pos == other.start_pos;
    }

    size_t get_file_pos(uint series_len, MtsNumChannelsT num_channels, MtsNumChannelsT channel = 0) const {
        return ((series_ind * num_channels + channel) * series_len + start_pos) * sizeof(float);
    }

    // Required for Cereal (de)serialization
    friend class cereal::access;

    template <class Archive>
    void serialize(Archive &ar) {
        ar(series_ind, start_pos, length);
    }
};

using DistanceT = double;

#endif  // TYPEDEFS_HPP

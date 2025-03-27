#ifndef TYPEDEFS_HPP
#define TYPEDEFS_HPP

#include <cassert>
#include <cstdint>
#include <fstream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <fftw3.h>
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

#ifdef USE_DOUBLE
using Real = double;
using fftwr_complex = fftwl_complex;
using fftwr_plan = fftwl_plan;
#define fftwr_malloc fftwl_malloc
#define fftwr_free fftwl_free
#define fftwr_execute fftwl_execute
#define fftwr_destroy_plan fftwl_destroy_plan
#define fftwr_plan_dft_1d fftwl_plan_dft_1d
#define fftwr_cleanup fftwl_cleanup
#define fftwr_forget_wisdom fftwl_forget_wisdom
#else
using Real = float;
using fftwr_complex = fftwf_complex;
using fftwr_plan = fftwf_plan;
#define fftwr_malloc fftwf_malloc
#define fftwr_free fftwf_free
#define fftwr_execute fftwf_execute
#define fftwr_destroy_plan fftwf_destroy_plan
#define fftwr_plan_dft_1d fftwf_plan_dft_1d
#define fftwr_cleanup fftwf_cleanup
#define fftwr_forget_wisdom fftwf_forget_wisdom
#endif

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
        return ((series_ind * num_channels + channel) * series_len + start_pos) * sizeof(Real);
    }

    // Required for Cereal (de)serialization
    friend class cereal::access;

    template <class Archive>
    void serialize(Archive &ar) {
        ar(series_ind, start_pos, length);
    }
};

#endif  // TYPEDEFS_HPP

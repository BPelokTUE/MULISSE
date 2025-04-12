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
    SaxSegIndT m_seg_ind;
    MtsNumChannelsT m_channel;

    bool operator==(const SaxSplitIndex &other) const {
        return m_seg_ind == other.m_seg_ind && m_channel == other.m_channel;
    }

    // Required for Cereal (de)serialization
    friend class cereal::access;

    template <class Archive>
    void serialize(Archive &ar) {
        ar(m_seg_ind, m_channel);
    }
};

struct SubsequenceInfo {
    /** @brief Index of the series within the file */
    uint m_series_ind;
    /** @brief Index of the start position of the subsequence within the series */
    uint m_start_pos;
    /** @brief Length of the subsequence */
    uint m_length;

    bool operator<(const SubsequenceInfo &other) const {
        return m_series_ind < other.m_series_ind ||
               (m_series_ind == other.m_series_ind && m_start_pos < other.m_start_pos);
    }

    bool operator==(const SubsequenceInfo &other) const {
        return m_series_ind == other.m_series_ind && m_start_pos == other.m_start_pos;
    }

    std::streampos get_file_pos(uint series_len, MtsNumChannelsT num_channels, MtsNumChannelsT channel = 0) const {
        return ((m_series_ind * num_channels + channel) * series_len + m_start_pos) * sizeof(Real);
    }

    // Required for Cereal (de)serialization
    friend class cereal::access;

    template <class Archive>
    void serialize(Archive &ar) {
        ar(m_series_ind, m_start_pos, m_length);
    }
};

#endif  // TYPEDEFS_HPP

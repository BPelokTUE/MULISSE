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

#ifdef USE_SINGLE_MASS
using MassT = float;
using fftwr_complex = fftwf_complex;
using fftwr_plan = fftwf_plan;
#define fftwr_malloc fftwf_malloc
#define fftwr_free fftwf_free
#define fftwr_execute fftwf_execute
#define fftwr_destroy_plan fftwf_destroy_plan
#define fftwr_plan_dft_1d fftwf_plan_dft_1d
#define fftwr_cleanup fftwf_cleanup
#define fftwr_forget_wisdom fftwf_forget_wisdom
#else
using MassT = double;
using fftwr_complex = fftw_complex;
using fftwr_plan = fftw_plan;
#define fftwr_malloc fftw_malloc
#define fftwr_free fftw_free
#define fftwr_execute fftw_execute
#define fftwr_destroy_plan fftw_destroy_plan
#define fftwr_plan_dft_1d fftw_plan_dft_1d
#define fftwr_cleanup fftw_cleanup
#define fftwr_forget_wisdom fftw_forget_wisdom
#endif

#ifdef USE_DOUBLE_FFT_PRE
using FftPrecT = double;
#else
using FftPrecT = float;
#endif

#ifdef USE_DOUBLE
using Real = double;
#else
using Real = float;
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

#endif  // TYPEDEFS_HPP

#ifndef UTIL_TYPES_NUMBERS_HPP
#define UTIL_TYPES_NUMBERS_HPP

#include <fftw3.h>

#include <cstdint>

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

#endif  // UTIL_TYPES_NUMBERS_HPP

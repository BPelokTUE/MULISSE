#ifndef FFT_ARRAY_HPP
#define FFT_ARRAY_HPP

#include <fftw3.h>

#include "Util/typedefs.hpp"

/** @brief A RAII wrapper for a fixed-size FFTW complex array */
class FftArray {
   public:
    FftArray() = default;

    explicit FftArray(size_t n);

    ~FftArray();

    FftArray(const FftArray& other);
    FftArray& operator=(const FftArray& other);

    // Move constructor
    FftArray(FftArray&& other) noexcept;

    fftwr_complex* data() noexcept;
    const fftwr_complex* data() const noexcept;

    fftwr_complex& operator[](size_t i);
    const fftwr_complex& operator[](size_t i) const;

    const size_t size() const noexcept;

   private:
    fftwr_complex* m_data;
    size_t m_size;
};

#endif  // FFT_ARRAY_HPP

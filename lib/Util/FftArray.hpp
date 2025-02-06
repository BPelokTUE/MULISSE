#ifndef FFT_ARRAY_HPP
#define FFT_ARRAY_HPP

#include <fftw3.h>

/** @brief A RAII wrapper for a fixed-size FFTW complex array */
class FftArray {
   public:
    explicit FftArray(size_t n);

    ~FftArray();

    FftArray(const FftArray&) = delete;
    FftArray& operator=(const FftArray&) = delete;

    fftw_complex* data() noexcept;
    const fftw_complex* data() const noexcept;

    fftw_complex& operator[](size_t i);
    const fftw_complex& operator[](size_t i) const;

   private:
    fftw_complex* m_data;
};

#endif  // FFT_ARRAY_HPP

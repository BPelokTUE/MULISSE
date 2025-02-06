#ifndef FFT_ARRAY_HPP
#define FFT_ARRAY_HPP

#include <fftw3.h>

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

    fftw_complex* data() noexcept;
    const fftw_complex* data() const noexcept;

    fftw_complex& operator[](size_t i);
    const fftw_complex& operator[](size_t i) const;

    const size_t size() const noexcept;

   private:
    fftw_complex* m_data;
    size_t m_size;
};

#endif  // FFT_ARRAY_HPP

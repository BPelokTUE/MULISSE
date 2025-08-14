#ifndef FFT_ARRAY_HPP
#define FFT_ARRAY_HPP

#include <fftw3.h>

#include <cstring>
#include <memory>

#include "Util/Types/FftTypes.hpp"

/** @brief A RAII wrapper for a fixed-size FFTW complex array */
class FftArray {
   public:
    FftArray() = default;

    explicit FftArray(size_t n) : m_data(nullptr), m_size(n) {
        m_data = static_cast<fftwr_complex*>(fftwr_malloc(n * sizeof(fftwr_complex)));
        if (!m_data) throw std::bad_alloc();

        for (size_t i = 0; i < n; ++i) {
            m_data[i][0] = 0;
            m_data[i][1] = 0;
        }
    }

    ~FftArray() { fftwr_free(m_data); }

    FftArray(const FftArray& other) : m_size(other.m_size) {
        m_data = static_cast<fftwr_complex*>(fftwr_malloc(sizeof(fftwr_complex) * m_size));
        if (!m_data) throw std::bad_alloc();
        std::memcpy(m_data, other.m_data, sizeof(fftwr_complex) * m_size);
    }

    FftArray& operator=(const FftArray& other) {
        if (this != &other) {
            fftwr_free(m_data);
            m_size = other.m_size;
            m_data = static_cast<fftwr_complex*>(fftwr_malloc(sizeof(fftwr_complex) * m_size));
            if (!m_data) throw std::bad_alloc();
            std::memcpy(m_data, other.m_data, sizeof(fftwr_complex) * m_size);
        }
        return *this;
    }

    // Move constructor
    FftArray(FftArray&& other) noexcept {
        other.m_data = nullptr;
        other.m_size = 0;
    }

    fftwr_complex* data() noexcept { return m_data; }
    const fftwr_complex* data() const noexcept { return m_data; }

    fftwr_complex& operator[](size_t i) { return m_data[i]; }
    const fftwr_complex& operator[](size_t i) const { return m_data[i]; }

    const size_t size() const noexcept { return m_size; }

   private:
    fftwr_complex* m_data;
    size_t m_size;
};

#endif  // FFT_ARRAY_HPP

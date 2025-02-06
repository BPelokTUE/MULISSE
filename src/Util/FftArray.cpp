#include <vector>

#include "Util/FftArray.hpp"

FftArray::FftArray(size_t n) : m_data(nullptr), m_size(n) {
    m_data = static_cast<fftw_complex*>(fftw_malloc(n * sizeof(fftw_complex)));
    if (!m_data) throw std::bad_alloc();

    for (size_t i = 0; i < n; ++i) {
        m_data[i][0] = 0;
        m_data[i][1] = 0;
    }
}

FftArray::~FftArray() { fftw_free(m_data); }

FftArray::FftArray(const FftArray& other) : m_size(other.m_size) {
    m_data = static_cast<fftw_complex*>(fftw_malloc(sizeof(fftw_complex) * m_size));
    if (!m_data) throw std::bad_alloc();
    std::memcpy(m_data, other.m_data, sizeof(fftw_complex) * m_size);
}

FftArray& FftArray::operator=(const FftArray& other) {
    if (this != &other) {
        fftw_free(m_data);
        m_size = other.m_size;
        m_data = static_cast<fftw_complex*>(fftw_malloc(sizeof(fftw_complex) * m_size));
        if (!m_data) throw std::bad_alloc();
        std::memcpy(m_data, other.m_data, sizeof(fftw_complex) * m_size);
    }
    return *this;
}

FftArray::FftArray(FftArray&& other) noexcept : m_data(other.m_data), m_size(other.m_size) {
    other.m_data = nullptr;
    other.m_size = 0;
}

fftw_complex* FftArray::data() noexcept { return m_data; }
const fftw_complex* FftArray::data() const noexcept { return m_data; }

fftw_complex& FftArray::operator[](size_t i) { return m_data[i]; }
const fftw_complex& FftArray::operator[](size_t i) const { return m_data[i]; }

const size_t FftArray::size() const noexcept { return m_size; }

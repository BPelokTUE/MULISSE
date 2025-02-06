#include <vector>

#include "Util/FftArray.hpp"

FftArray::FftArray(size_t n) : m_data(nullptr) {
    m_data = static_cast<fftw_complex*>(fftw_malloc(n * sizeof(fftw_complex)));
    if (!m_data) throw std::bad_alloc();
}

FftArray::~FftArray() { fftw_free(m_data); }

fftw_complex* FftArray::data() noexcept { return m_data; }
const fftw_complex* FftArray::data() const noexcept { return m_data; }

fftw_complex& FftArray::operator[](size_t i) { return m_data[i]; }
const fftw_complex& FftArray::operator[](size_t i) const { return m_data[i]; }

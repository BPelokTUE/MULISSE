#ifndef ENVELOPE_HPP
#define ENVELOPE_HPP

#include "typedefs.hpp"

/**
 * @brief Envelope of a multivariate time series
 *
 * An envelope is a set of lower and upper bounds that summarizes subsequences of a multivariate time series
 */
struct Envelope {
    /** @brief Lower bounds of the envelope */
    vec<float> lower;
    /** @brief Upper bounds of the envelope */
    vec<float> upper;

    /**
     * @brief Get the size (number of entries) of the envelope
     *
     * @return The size of the envelope
     * */
    size_t size() const;

    /**
     * @brief Resize the envelope
     *
     * @param new_size The new size of the envelope
     * */
    void resize(size_t new_size);
};

#endif  // ENVELOPE_HPP

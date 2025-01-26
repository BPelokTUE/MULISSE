#ifndef ENVELOPE_HPP
#define ENVELOPE_HPP

#include "typedefs.hpp"

class Envelope {
   public:
    vec<float> lower, upper;

    size_t size() const;

    bool operator==(const Envelope& other) const;
};

#endif  // ENVELOPE_HPP

#include "Summarization/Envelope.hpp"

#include <iostream>

size_t Envelope::size() const { return lower.size(); }

void Envelope::resize(size_t new_size) {
    lower.resize(new_size);
    upper.resize(new_size);
}

bool Envelope::operator==(const Envelope& other) const { return lower == other.lower && upper == other.upper; }

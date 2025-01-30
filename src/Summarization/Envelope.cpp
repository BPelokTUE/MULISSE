#include "Summarization/Envelope.hpp"

#include <iostream>

size_t Envelope::size() const { return lower.size(); }

void Envelope::resize(size_t new_size) {
    lower.resize(new_size);
    upper.resize(new_size);
}

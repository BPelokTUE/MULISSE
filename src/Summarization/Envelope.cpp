#include "Summarization/Envelope.hpp"

#include <iostream>

size_t Envelope::size() const { return lower.size(); }

bool Envelope::operator==(const Envelope& other) const { return lower == other.lower && upper == other.upper; }

#ifndef UTIL_STATS_ENVELOPESTATSUTIL_HPP
#define UTIL_STATS_ENVELOPESTATSUTIL_HPP

#include "Util/Types/Numbers.hpp"
#include "Util/Types/Pointers.hpp"

class EnvelopeEntryGenerator;

uptr<EnvelopeEntryGenerator> get_simple_envelope_entry_generator(uint segment_len, bool normalized = true);

#endif  // UTIL_STATS_ENVELOPESTATSUTIL_HPP

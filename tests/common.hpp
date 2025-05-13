#ifndef TESTS_COMMON_HPP
#define TESTS_COMMON_HPP

#include <doctest/doctest.h>

#include <algorithm>

#include "Index/Entry/Envelope.hpp"
#include "Index/Entry/IndexEntry.hpp"
#include "Index/Entry/Paa.hpp"

void require_paa_entries_equal(const vec<IndexEntry<Paa>> &actual, const vec<IndexEntry<Paa>> &expected);

void require_sorted_paa_entries_equal(vec<IndexEntry<Paa>> &expected, vec<IndexEntry<Paa>> &actual);

void require_envelope_entries_equal(vec<IndexEntry<Envelope>> &expected, vec<IndexEntry<Envelope>> &actual);

void require_sorted_envelope_entries_equal(vec<IndexEntry<Envelope>> &expected, vec<IndexEntry<Envelope>> &actual);

#endif  // TESTS_COMMON_HPP

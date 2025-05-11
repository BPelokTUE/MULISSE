#ifndef INDEX_TRAITS_INDEXTRAITS_HPP
#define INDEX_TRAITS_INDEXTRAITS_HPP

#include "Index/Entry/Envelope.hpp"
#include "Index/Entry/Paa.hpp"
#include "Index/Traits/FinalizedTraits.hpp"

template <typename T>
struct IndexTraits;

template <>
struct IndexTraits<Paa> {
    using FinalizedTag = PaaTag;
};

template <>
struct IndexTraits<Envelope> {
    using FinalizedTag = EnvelopeTag;
};

#endif  // INDEX_TRAITS_INDEXTRAITS_HPP

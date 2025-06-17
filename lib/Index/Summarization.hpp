#ifndef INDEX_SUMMARIZATION_HPP
#define INDEX_SUMMARIZATION_HPP

#include "Util/Types/Containers.hpp"
#include "Util/Types/Numbers.hpp"
#include "Util/Types/Pointers.hpp"

template <typename T>
class IndexEntry;

template <typename T>
class IEntryGenerator;

template <typename T>
class IEntryMerger;

template <typename T>
vec<vec<IndexEntry<T>>> summarize_dataset(uint num_length_groups, uint num_series, const vec<uint> &mts_inds,
                                          uptr<IEntryGenerator<T>> generator, uptr<IEntryMerger<T>> merger);

#endif  // INDEX_SUMMARIZATION_HPP

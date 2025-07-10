#ifndef UTIL_HELPERFUNCS_PARALLELSORT_HPP
#define UTIL_HELPERFUNCS_PARALLELSORT_HPP

#include <algorithm>
#ifndef DISABLE_PARALLELISM
#include <tbb/parallel_sort.h>
#endif

template <typename IT>
void parallel_sort(IT begin, IT end) {
#ifdef DISABLE_PARALLELISM
    std::sort(begin, end);
#else
    tbb::parallel_sort(begin, end);
#endif
}

#endif  // UTIL_HELPERFUNCS_PARALLELSORT_HPP

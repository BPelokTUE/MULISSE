#ifndef UTIL_STATS_SCOREFUNC_INDEXSTATSEXTRACTOR_HPP
#define UTIL_STATS_SCOREFUNC_INDEXSTATSEXTRACTOR_HPP

#include "Util/Types/Containers.hpp"
#include "Util/Types/Numbers.hpp"

struct IndexStats;

class IndexStatsExtractor {
   public:
    virtual ~IndexStatsExtractor() = default;

    /**
     * @brief Extract the statistics from the index
     * @param stats The statistics of the index
     * @return The extracted statistics
     */
    virtual vec<Real> extract(const IndexStats &stats) const = 0;
};

#endif UTIL_STATS_SCOREFUNC_INDEXSTATSEXTRACTOR_HPP

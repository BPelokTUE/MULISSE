#ifndef UTIL_STATS_SCOREFUNC_INDEXSTATSEXTRACTOR_HPP
#define UTIL_STATS_SCOREFUNC_INDEXSTATSEXTRACTOR_HPP

#include "Util/Types/Numbers.hpp"
#include "Util/Types/Vec.hpp"

struct IndexStats;

class IIndexStatsExtractor {
   public:
    virtual ~IIndexStatsExtractor() = default;

    /**
     * @brief Extract the statistics from the index
     * @param stats The statistics of the index
     * @return The extracted statistics
     */
    virtual vec<Real> extract(const IndexStats &stats) const = 0;
};

#endif  // UTIL_STATS_SCOREFUNC_INDEXSTATSEXTRACTOR_HPP

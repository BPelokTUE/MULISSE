#ifndef UTIL_STATS_INDEXSTATSEXTRACTOR_ENVELOPESTDEXTRACTOR_HPP
#define UTIL_STATS_INDEXSTATSEXTRACTOR_ENVELOPESTDEXTRACTOR_HPP

#include "Util/Stats/IndexStatsExtractor/IndexStatsExtractor.hpp"

/**
 * @brief Extracts the cross-dataset mean of mean envelope widths (ranges) and the standard deviations of the
 * within-series means and stds of the lower and upper envelope bounds and of mid=(lower+upper)/2, that is [mu_l,
 * sigma_l, mu_u, sigma_u, mu_m, sigma_m]
 */
class EnvelopeShapeExtractor : public IIndexStatsExtractor {
   public:
    /**
     * @brief Get stats describing the shape of the envelopes in the index
     * @param stats The statistics of the index
     */
    vec<Real> extract(const IndexStats &stats) const override;
};

#endif  // UTIL_STATS_INDEXSTATSEXTRACTOR_ENVELOPESTDEXTRACTOR_HPP

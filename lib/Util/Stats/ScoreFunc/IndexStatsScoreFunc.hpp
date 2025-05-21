#ifndef UTIL_STATS_INDEXSTATSSCOREFUNC_HPP
#define UTIL_STATS_INDEXSTATSSCOREFUNC_HPP

struct IndexStats;
class IScoreFunc;
class IIndexStatsExtractor;

#include "Util/Types/Numbers.hpp"
#include "Util/Types/Pointers.hpp"

/** @brief Wrapper around an IScoreFunc that gets the score of an index based on the specified statistics */
class IndexStatsScoreFunc {
   public:
    /**
     * @brief Construct a new IndexStatsScoreFunc object
     * @param stats_extractor The function to extract the relevant index statistics
     * @param score_func The score function to use
     */
    IndexStatsScoreFunc(uptr<IIndexStatsExtractor> stats_extractor, uptr<IScoreFunc> score_func);

    Real calculate_score(const IndexStats &stats) const;

   private:
    uptr<IScoreFunc> m_score_func;
    uptr<IIndexStatsExtractor> m_stats_extractor;
};

#endif  // UTIL_STATS_INDEXSTATSSCOREFUNC_HPP

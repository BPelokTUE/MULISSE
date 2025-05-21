#include "Util/Stats/ScoreFunc/IndexStatsScoreFunc.hpp"

#include "Util/Stats/IndexStatsExtractor/IndexStatsExtractor.hpp"
#include "Util/Stats/ScoreFunc/ScoreFunc.hpp"

IndexStatsScoreFunc::IndexStatsScoreFunc(uptr<IIndexStatsExtractor> stats_extractor, uptr<IScoreFunc> score_func)
    : m_score_func(std::move(score_func)), m_stats_extractor(std::move(stats_extractor)) {}

Real IndexStatsScoreFunc::calculate_score(const IndexStats &stats) const {
    return m_score_func->calculate_score(m_stats_extractor->extract(stats));
}

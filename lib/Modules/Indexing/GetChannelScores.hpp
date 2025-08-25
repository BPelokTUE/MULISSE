#ifndef MODULES_INDEXING_STRATEGYFACTORY_GETCHANNELSCORES_HPP
#define MODULES_INDEXING_STRATEGYFACTORY_GETCHANNELSCORES_HPP

#include "Util/Types/Numbers.hpp"
#include "Util/Types/Vec.hpp"

class GeneralIndexProperties;

/**
 * @brief Get the score for each channel using the IEnvelopeScoreFunc specified in the index options.
 * @param opts The index options containing the segmentation parameters.
 * @return A vector of scores for each channel, or an empty vector if score-based channel segmentation strategy is not
 * used.
 */
vec<Real> get_channel_scores(const GeneralIndexProperties &opts);

#endif  // MODULES_INDEXING_STRATEGYFACTORY_GETCHANNELSCORES_HPP

#ifndef ENUMS_CHANNELSEGMENTATIONSTRATEGYTYPE_HPP
#define ENUMS_CHANNELSEGMENTATIONSTRATEGYTYPE_HPP

#include "Util/HelperFuncs/Enums.hpp"

/** @brief Enum for IChannelSegmentationStrategy implementations */
enum class ChannelSegmentationStrategyType { SINGLE, MULTI, SCORE_BASED };

using CHSS = ChannelSegmentationStrategyType;

DEFINE_ENUM_CONSTS(CHSS, CHANNEL_SEGMENTATION_STRATEGY, false, (umap<str, CHSS>{{"scored", CHSS::SCORE_BASED}}));

#endif  // ENUMS_CHANNELSEGMENTATIONSTRATEGYTYPE_HPP

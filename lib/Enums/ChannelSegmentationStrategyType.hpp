#ifndef ENUMS_CHANNELSEGMENTATIONSTRATEGYTYPE_HPP
#define ENUMS_CHANNELSEGMENTATIONSTRATEGYTYPE_HPP

#include "Util/HelperFuncs/Enums.hpp"

/** @brief Enum for IChannelSegmentationStrategy implementations */
enum class ChannelSegmentationStrategyType { SINGLE, MULTI, SCORE_BASED };

DEFINE_ENUM_CONSTS(ChannelSegmentationStrategyType, CHANNEL_SEGMENTATION_STRATEGY, false,
                   (umap<str, ChannelSegmentationStrategyType>{
                       {"scored", ChannelSegmentationStrategyType::SCORE_BASED}}));

#endif  // ENUMS_CHANNELSEGMENTATIONSTRATEGYTYPE_HPP

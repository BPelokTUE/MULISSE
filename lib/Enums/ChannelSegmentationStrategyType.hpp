#ifndef ENUMS_CHANNELSEGMENTATIONSTRATEGYTYPE_HPP
#define ENUMS_CHANNELSEGMENTATIONSTRATEGYTYPE_HPP

#include "Util/HelperFuncs/Enums.hpp"

/** @brief Enum for IChannelSegmentationStrategy implementations */
enum class ChannelSegmentationStrategyType { SINGLE, MULTI, ENV_STATS_BASED, ENV_WIDTH_BASED };

using CHSS = ChannelSegmentationStrategyType;

DEFINE_ENUM_CONSTS(CHSS, CHANNEL_SEGMENTATION_STRATEGY, false,
                   (umap<str, CHSS>{{"env_stats", CHSS::ENV_STATS_BASED}, {"env_width", CHSS::ENV_WIDTH_BASED}}));

constexpr std::array SAMPLING_CH_SEGMENTATION_STRATEGY_TYPES = {CHSS::ENV_STATS_BASED, CHSS::ENV_WIDTH_BASED};

#endif  // ENUMS_CHANNELSEGMENTATIONSTRATEGYTYPE_HPP

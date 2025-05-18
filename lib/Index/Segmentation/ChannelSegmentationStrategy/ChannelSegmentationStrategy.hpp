#ifndef INDEX_SEGMENTATION_CHANNELSEGMENTATIONSTRATEGY_CHANNELSEGMENTATIONSTRATEGY_HPP
#define INDEX_SEGMENTATION_CHANNELSEGMENTATIONSTRATEGY_CHANNELSEGMENTATIONSTRATEGY_HPP

#include <cereal/access.hpp>

#include "Enums/ChannelSegmentationStrategyType.hpp"
#include "Util/Types/Pointers.hpp"

class ISegmentationStrategy;

class IChannelSegmentationStrategy {
   public:
    virtual ~IChannelSegmentationStrategy() = default;

    /**
     * @brief Get the segmentation strategy for the specified channel.
     * @param channel_ind The channel index.
     * @return The segmentation strategy for the specified channel.
     */
    virtual sptr<ISegmentationStrategy> get_segmentation_strategy(uint channel_ind) const = 0;

    /**
     * @brief Get a const pointer to the segmentation strategy for the specified channel.
     * @param channel_ind The channel index.
     * @return A const pointer to the segmentation strategy for the specified channel.
     */
    virtual const ISegmentationStrategy *get_const_segmentation_strategy(uint channel_ind) const = 0;

    /**
     * @brief Get the type of channel segmentation strategy.
     * @return The type of channel segmentation strategy.
     */
    virtual ChannelSegmentationStrategyType get_type() const = 0;
};

#endif  // INDEX_SEGMENTATION_CHANNELSEGMENTATIONSTRATEGY_CHANNELSEGMENTATIONSTRATEGY_HPP

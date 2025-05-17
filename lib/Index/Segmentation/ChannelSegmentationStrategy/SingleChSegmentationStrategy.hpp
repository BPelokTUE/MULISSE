#ifndef INDEX_SEGMENTATION_CHANNELSEGMENTATIONSTRATEGY_SINGLECHSEGMENTATIONSTRATEGY_HPP
#define INDEX_SEGMENTATION_CHANNELSEGMENTATIONSTRATEGY_SINGLECHSEGMENTATIONSTRATEGY_HPP

#include "Index/Segmentation/ChannelSegmentationStrategy/ChannelSegmentationStrategy.hpp"

/** IChannelSegmentationStrategy implementation that uses the same strategy for each channel */
class SingleChSegmentationStrategy : public IChannelSegmentationStrategy {
   public:
    /**
     * @brief Construct a new SingleChSegmentationStrategy with the given segmentation strategy.
     * @param segmentation_strategy The segmentation strategy to use for all channels.
     */
    SingleChSegmentationStrategy(sptr<ISegmentationStrategy> segmentation_strategy);

    SingleChSegmentationStrategy() = default;

    sptr<ISegmentationStrategy> get_segmentation_strategy(uint ch_ind) const override;

    const ISegmentationStrategy *get_const_segmentation_strategy(uint ch_ind) const override;

    ChannelSegmentationStrategyType get_type() const override;

   private:
    sptr<ISegmentationStrategy> m_segmentation_strategy;

    // Required for Cereal (de)serialization
    friend class cereal::access;

    template <class Archive>
    void serialize(Archive &ar) {
        ar(m_segmentation_strategy);
    }
};

#endif  //  INDEX_SEGMENTATION_CHANNELSEGMENTATIONSTRATEGY_SINGLECHSEGMENTATIONSTRATEGY_HPP

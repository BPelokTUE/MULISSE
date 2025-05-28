#ifndef INDEX_SEGMENTATION_CHANNELSEGMENTATIONSTRATEGY_MULTICHSEGMENTATIONSTRATEGY_HPP
#define INDEX_SEGMENTATION_CHANNELSEGMENTATIONSTRATEGY_MULTICHSEGMENTATIONSTRATEGY_HPP

#include "Index/Segmentation/ChannelSegmentationStrategy/ChannelSegmentationStrategy.hpp"
#include "Util/Types/Containers.hpp"
#include "Util/Types/Numbers.hpp"

class MultiChSegmentationStrategy : public IChannelSegmentationStrategy {
   public:
    /**
     * @brief Constructor for MultiChSegmentationStrategy.
     * @param segmentation_strategies Vector of segmentation strategies for each channel.
     */
    MultiChSegmentationStrategy(vec<sptr<ISegmentationStrategy>> segmentation_strategies);

    /**
     * @brief Constructor for MultiChSegmentationStrategy.
     * @param num_seg_to_strategy Function to convert number of segments to segmentation strategy.
     * @param avg_num_segments Average number of segments across all channels.
     * @param num_seg_props_path Path to the file containing the proportion of segments to use per channel.
     */
    MultiChSegmentationStrategy(std::function<sptr<ISegmentationStrategy>(SaxSegIndT)> num_seg_to_strategy,
                                SaxSegIndT avg_num_segments, const str num_seg_props_path);

    MultiChSegmentationStrategy() = default;

    sptr<ISegmentationStrategy> get_segmentation_strategy(uint channel_ind) const override;

    const ISegmentationStrategy *get_const_segmentation_strategy(uint channel_ind) const override;

    ChannelSegmentationStrategyType get_type() const override;

   protected:
    vec<sptr<ISegmentationStrategy>> m_segmentation_strategies;

    // Required for Cereal (de)serialization
    friend class cereal::access;

    template <class Archive>
    void serialize(Archive &ar) {
        ar(m_segmentation_strategies);
    };
};

#endif  // INDEX_SEGMENTATION_CHANNELSEGMENTATIONSTRATEGY_MULTICHSEGMENTATIONSTRATEGY_HPP

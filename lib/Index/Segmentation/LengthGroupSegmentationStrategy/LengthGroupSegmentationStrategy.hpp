#ifndef LENGTH_GROUP_SEGMENTATION_STRATEGY_HPP
#define LENGTH_GROUP_SEGMENTATION_STRATEGY_HPP

#include "Enums/LengthGroupSegmentationStrategyType.hpp"
#include "Index/Segmentation/SegmentationStrategy/SegmentationStrategy.hpp"
#include "Util/RunSettings/LengthProperties.hpp"
#include "Util/Types/Numbers.hpp"
#include "Util/Types/Pointers.hpp"

/** @brief Interface for length group segmentation strategies. */
class ILengthGroupSegmentationStrategy {
   public:
    virtual ~ILengthGroupSegmentationStrategy() = default;

    /**
     * @brief Get the segmentation strategy for the specified length group.
     * @param lg_ind The length group index.
     * @return The segmentation strategy for the specified length group.
     */
    virtual sptr<ISegmentationStrategy> get_segmentation_strategy(uint lg_ind) const = 0;

    /**
     * @brief Get a const pointer to the segmentation strategy for the specified length group.
     * @param lg_ind The length group index.
     * @return A const pointer to the segmentation strategy for the specified length group.
     */
    virtual const ISegmentationStrategy *get_const_segmentation_strategy(uint lg_ind) const = 0;

    /**
     * @brief Get the type of length group segmentation strategy.
     * @return The type of length group segmentation strategy.
     */
    virtual LengthGroupSegmentationStrategyType get_type() const = 0;
};

#endif  // LENGTH_GROUP_SEGMENTATION_STRATEGY_HPP

#ifndef LENGTH_GROUP_SEGMENTATION_STRATEGY_HPP
#define LENGTH_GROUP_SEGMENTATION_STRATEGY_HPP

#include "Enums/LengthGroupSegmentationStrategyType.hpp"
#include "Index/Segmentation/SegmentationStrategy.hpp"
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

/** @brief ILengthGroupSegmentationStrategy implementation that uses the same SegmentationStrategy for all groups. */
class SingleSegmentationStrategy : public ILengthGroupSegmentationStrategy {
   public:
    /**
     * @brief Construct a new SingleSegmentationStrategy with the given segmentation strategy.
     * @param segmentation_strategy The segmentation strategy to use for all groups.
     */
    SingleSegmentationStrategy(sptr<ISegmentationStrategy> &&segmentation_strategy);

    sptr<ISegmentationStrategy> get_segmentation_strategy(uint lg_ind) const override;

    const ISegmentationStrategy *get_const_segmentation_strategy(uint lg_ind) const override;

    LengthGroupSegmentationStrategyType get_type() const override;

   private:
    sptr<ISegmentationStrategy> m_segmentation_strategy;
};

/** @brief ILengthGroupSegmentationStrategy implementation that uses different SegmentationStrategy for each group,
 * parametrized by the length range of the group. */
class MultiSegmentationStrategy : public ILengthGroupSegmentationStrategy {
   public:
    /**
     * @brief Construct a new MultiSegmentationStrategy with the given a segmentation strategy factory.
     * @param segmentation_strategy_factory The factory function to create segmentation strategies for each group.
     */
    MultiSegmentationStrategy(std::function<sptr<ISegmentationStrategy>(uint, uint)> segmentation_strategy_factory);

    MultiSegmentationStrategy() = default;

    sptr<ISegmentationStrategy> get_segmentation_strategy(uint lg_ind) const override;

    const ISegmentationStrategy *get_const_segmentation_strategy(uint lg_ind) const override;

    LengthGroupSegmentationStrategyType get_type() const override;

   protected:
    vec<sptr<ISegmentationStrategy>> m_segmentation_strategies;
};

/** @brief ILengthGroupSegmentationStrategy implementation that uses different SegmentationStrategy for each group,
 * parametrized by the length range of the group, and adapted based on the total "presence" of the group. */
class AdaptiveMultiSegmentationStrategy : public MultiSegmentationStrategy {
   public:
    /**
     * @brief Construct a new AdaptiveMultiSegmentationStrategy with the given segmentation strategy factory.
     * @param segmentation_strategy_factory The factory function to create segmentation strategies for each group.
     * @param avg_num_segments The average number of segments for each group.
     * @param pos_per_env The number of positions per envelope.
     */
    AdaptiveMultiSegmentationStrategy(
        std::function<sptr<ISegmentationStrategy>(uint, uint, SaxSegIndT)> segmentation_strategy_factory,
        SaxSegIndT avg_num_segments, uint pos_per_env);

    LengthGroupSegmentationStrategyType get_type() const override;

   private:
    /**
     * @brief Calculate the number of segments for each length group
     * @param length_props Length properties
     * @param pos_per_env Number of positions per envelope
     * @param avg_num_segments Average number of segments
     */
    vec<SaxSegIndT> calculate_num_segments_per_lg(const LengthProperties &length_props, uint pos_per_env,
                                                  SaxSegIndT avg_num_segments);
};

#endif  // LENGTH_GROUP_SEGMENTATION_STRATEGY_HPP

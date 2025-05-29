#ifndef INDEX_SEGMENTATION_CHANNELSEGMENTATIONSTRATEGY_SCOREBASEDCHSEGMENTATIONSTRATEGY_HPP
#define INDEX_SEGMENTATION_CHANNELSEGMENTATIONSTRATEGY_SCOREBASEDCHSEGMENTATIONSTRATEGY_HPP

#include "Index/Segmentation/ChannelSegmentationStrategy/SamplingChSegmentationStrategy.hpp"
#include "Util/HelperFuncs/Conversion.hpp"
#include "Util/Types/Numbers.hpp"

class IEnvelopeScores;
class IScoreToSegmentationStrategy;

class ScoreBasedChSegmentationStrategy : public SamplingChSegmentationStrategy {
   public:
    ~ScoreBasedChSegmentationStrategy();

    ScoreBasedChSegmentationStrategy();

    /**
     * @brief Constructor for ScoreBasedChSegmentationStrategy.
     * @param envelope_scores Used for calculating envelope scores
     * @param score_to_segmentation_strategy Function for converting scores to segmentation strategies
     * @param sampling_params Parameters for sampling
     */
    ScoreBasedChSegmentationStrategy(uptr<IEnvelopeScores> envelope_score,
                                     uptr<IScoreToSegmentationStrategy> score_to_segmentation_strategy,
                                     SamplingChSSSamplingParams sampling_params);

   protected:
    void initialize_segmentation_strategies() override;

   private:
    uptr<IEnvelopeScores> m_envelope_scores;
    uptr<IScoreToSegmentationStrategy> m_score_to_segmentation_strategy;
};

#endif  // INDEX_SEGMENTATION_CHANNELSEGMENTATIONSTRATEGY_SCOREBASEDCHSEGMENTATIONSTRATEGY_HPP

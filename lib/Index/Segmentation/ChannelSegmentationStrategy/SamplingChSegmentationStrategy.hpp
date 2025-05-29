#ifndef INDEX_SEGMENTATION_CHANNELSEGMENTATIONSTRATEGY_SAMPLINGCHSEGMENTATIONSTRATEGY_HPP
#define INDEX_SEGMENTATION_CHANNELSEGMENTATIONSTRATEGY_SAMPLINGCHSEGMENTATIONSTRATEGY_HPP

#include "Index/Segmentation/ChannelSegmentationStrategy/MultiChSegmentationStrategy.hpp"
#include "Util/Types/Containers.hpp"

class IScoreToSegmentationStrategy;
class EnvelopeEntryGenerator;
class SamplingChSSSamplingParams;

struct SamplingChSSLateInitParams {
    MtsNumChannelsT m_num_channels;
    uint m_sample_size, m_series_len;
    const str* m_dataset_path;
    vec<vec<uint>> m_mts_inds;
    uptr<EnvelopeEntryGenerator> m_generator;

    ~SamplingChSSLateInitParams();
};

class SamplingChSegmentationStrategy : public MultiChSegmentationStrategy {
   public:
    SamplingChSegmentationStrategy() = default;

   protected:
    /**
     * @brief Late-initialize the strategy
     * @param sampling_params Parameters for sampling
     */
    void initialize(SamplingChSSSamplingParams sampling_params);

    virtual void initialize_segmentation_strategies() = 0;

    uptr<SamplingChSSLateInitParams> m_late_init_params;
};

#endif  // INDEX_SEGMENTATION_CHANNELSEGMENTATIONSTRATEGY_SAMPLINGCHSEGMENTATIONSTRATEGY_HPP

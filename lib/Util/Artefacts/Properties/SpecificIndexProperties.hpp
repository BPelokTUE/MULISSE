#ifndef INDEX_INDEXPARAMS_HPP
#define INDEX_INDEXPARAMS_HPP

#include "Enums/ChannelSegmentationStrategyType.hpp"
#include "Enums/EntryMergerType.hpp"
#include "Enums/EnvelopeScoresTypes.hpp"
#include "Enums/IndexType.hpp"
#include "Enums/LengthGroupSegmentationStrategyType.hpp"
#include "Enums/SaxBreakpointStrategyType.hpp"
#include "Enums/SegmentationStrategyType.hpp"
#include "Enums/iSaxSplitStrategyType.hpp"
#include "Index/EnvelopeIndex/EnvelopeParams.hpp"
#include "Index/Segmentation/ChannelSegmentationStrategy/ScoreBased/ScoreBasedChSSParams.hpp"
#include "Util/Constants/Sax.hpp"
#include "Util/Types/Numbers.hpp"

/** @brief Interface for specific index properties */
struct ISpecificIndexProperties {
    virtual ~ISpecificIndexProperties() = default;

    /**
     * @brief Get the type of the index
     * @return The type of the index
     */
    virtual IndexType get_type() const = 0;
};

/** @brief Segmentation properties of indexes */
struct SegmentationProperties {
    /** @brief Number of segments to use */
    SaxSegIndT m_num_segments;
    /** @brief Type of strategy to use for length group segmentation */
    LengthGroupSegmentationStrategyType m_lg_strategy_type = SINGLE;
    /** @brief Type of strategy to use for channel segmentation */
    ChannelSegmentationStrategyType m_ch_strategy_type = ChannelSegmentationStrategyType::SINGLE;
    /** @brief Type of strategy to use for segmentation */
    SegmentationStrategyType m_strategy_type = UNIFORM;
    /** @brief File containing the proportions of segments  */
    str m_ch_num_seg_props_file = "";
    /** @brief Parameters of ScoreBasedChSegmentationStrategy */
    const ScoreBasedChSSParams *m_score_based_chss_params = nullptr;
};

/** @brief SAX representation properties of indexes */
struct SaxProperties {
    /** @brief Number of bits to use for the SAX breakpoints */
    SaxNumBitsT m_num_bits = MAX_NUM_BITS_LIMIT;
    /** @brief Strategy for getting the breakpoints of the symbol intervals */
    SaxBreakpointStrategyType m_breakpoint_strategy_type = EQUIPROBABLE;
    /** @brief Only used for FixedBreakpointStrategy: path to the plain text file to load the fixed breakpoints from */
    str m_breakpoints_file = "";
};

/** @brief Merger properties of indexes */
struct MergerProperties {
    /** @brief Type of entry merger to use */
    EntryMergerType m_entry_merger_type;
    /** @brief SAX parameters for SAX-based mergers, nullptr for non-SAX-based mergers */
    SaxProperties *m_merger_sax_props;
};

/** @brief iSAX trie properties of indexes */
struct iSaxTrieProperties {
    /** @brief Whether to merge entries in the leaves of the tree */
    bool m_merge_in_leaves = false;
    /** @brief Only used for EntropyMaximizingSplitStrategy: whether to select the segment with the min number of bits
     * in case of a tie */
    bool m_min_num_bits_on_tie = true;
    /** @brief Starting number of bits per segment */
    SaxNumBitsT m_first_layer_num_bits = 1;
    /** @brief Strategy for choosing the index to split on */
    iSaxSplitStrategyType m_split_strategy_type = iSaxSplitStrategyType::ENTROPY_MAXIMIZING;
    /** @brief Maximum number of entries in a leaf */
    size_t m_leaf_capacity;
};

// ----------------------------------------------------------------- //

/** @brief Properties for PAA segmentation-based indexes */
struct PaaIndexProperties : virtual ISpecificIndexProperties {
    /** @brief segmentation parameters */
    SegmentationProperties m_segmentation_params;
    /** @brief merger parameters */
    MergerProperties m_merger_params;

    /**
     * @brief Constructor
     * @param segmentation_params Segmentation parameters
     * @param merger_params Merger parameters
     */
    PaaIndexProperties(SegmentationProperties segmentation_params, MergerProperties merger_params)
        : m_segmentation_params(segmentation_params), m_merger_params(merger_params) {}
};

/** @brief Parameters for indexes that use SAX */
struct SaxIndexProperties : virtual PaaIndexProperties {
    /** @brief SAX parameters */
    SaxProperties m_sax_params;

    /**
     * @brief Constructor
     * @param segmentation_params Segmentation parameters
     * @param merger_params Merger parameters
     * @param sax_params SAX parameters
     */
    SaxIndexProperties(SegmentationProperties segmentation_params, MergerProperties merger_params,
                       SaxProperties sax_params)
        : PaaIndexProperties(segmentation_params, merger_params), m_sax_params(sax_params) {}
};

/** @brief Parameters for indexes that use envelopes */
struct EnvelopeIndexProperties : virtual PaaIndexProperties {
    IndexType get_type() const override { return ENVELOPE; }

    /** @brief Size of the starting position groups */
    uint m_pos_per_env;

    /**
     * @brief Constructor
     * @param segmentation_params Segmentation parameters
     * @param merger_params Merger parameters
     * @param pos_per_env Size of the starting position groups
     */
    EnvelopeIndexProperties(SegmentationProperties segmentation_params, MergerProperties merger_params,
                            uint pos_per_env)
        : PaaIndexProperties(segmentation_params, merger_params), m_pos_per_env(pos_per_env) {}

    /**
     * @brief Set the relevant flat envelope parameters (num_segments, pos_per_env)
     * @param flat_envelope_params The flat envelope parameters
     */
    void set_flat_envelope_params(const EnvelopeParams &flat_envelope_params) {
        m_pos_per_env = flat_envelope_params.m_pos_per_env;
        m_segmentation_params.m_num_segments = flat_envelope_params.m_num_segments;
    }
};

/** @brief Parameters for SAX + Envelope indexes */
struct SaxEnvelopeIndexProperties : virtual EnvelopeIndexProperties, virtual SaxIndexProperties {
    IndexType get_type() const override { return SAX_ENVELOPE; }

    /**
     * @brief Constructor
     * @param segmentation_params Segmentation parameters
     * @param merger_params Merger parameters
     * @param pos_per_env Size of the starting position groups
     * @param sax_params iSAX parameters
     */
    SaxEnvelopeIndexProperties(SegmentationProperties segmentation_params, MergerProperties merger_params,
                               uint pos_per_env, SaxProperties sax_params)
        : PaaIndexProperties(segmentation_params, merger_params),
          EnvelopeIndexProperties(segmentation_params, merger_params, pos_per_env),
          SaxIndexProperties(segmentation_params, merger_params, sax_params) {}
};

/** @brief Parameters for iSAX indexes */
struct iSaxIndexProperties : virtual PaaIndexProperties, virtual SaxIndexProperties {
    /** @brief iSAX trie parameters */
    iSaxTrieProperties m_isax_trie_params;

    IndexType get_type() const override { return ISAX; }

    /**
     * @brief Constructor
     * @param segmentation_params Segmentation parameters
     * @param merger_params Merger parameters
     * @param sax_params SAX parameters
     * @param isax_trie_params iSAX trie parameters
     */
    iSaxIndexProperties(SegmentationProperties segmentation_params, MergerProperties merger_params,
                        SaxProperties sax_params, iSaxTrieProperties isax_trie_params)
        : PaaIndexProperties(segmentation_params, merger_params),
          SaxIndexProperties(segmentation_params, merger_params, sax_params),
          m_isax_trie_params(isax_trie_params) {}
};

/** @brief Parameters for iSAX + Envelope indexes */
struct iSaxEnvelopeIndexProperties : virtual EnvelopeIndexProperties, virtual iSaxIndexProperties {
    IndexType get_type() const override { return ISAX_ENVELOPE; }

    /**
     * @brief Constructor
     * @param segmentation_params Segmentation parameters
     * @param merger_params Merger parameters
     * @param pos_per_env Size of the starting position groups
     * @param enveloping_params Envelope parameters
     * @param sax_params SAX parameters
     * @param isax_trie_params iSAX trie parameters
     */
    iSaxEnvelopeIndexProperties(SegmentationProperties segmentation_params, MergerProperties merger_params,
                                uint pos_per_env, SaxProperties sax_params, iSaxTrieProperties isax_trie_params)
        : PaaIndexProperties(segmentation_params, merger_params),
          EnvelopeIndexProperties(segmentation_params, merger_params, pos_per_env),
          SaxIndexProperties(segmentation_params, merger_params, sax_params),
          iSaxIndexProperties(segmentation_params, merger_params, sax_params, isax_trie_params) {}
};

/** @brief Properties for indexes that group envelopes */
struct EnvelopeGroupingProperties {
    /** @brief Whether to use invSAX sorting or not */
    bool m_use_inv_sax_sorting;
    /** @brief Whether to group envelopes per series or not */
    bool m_group_per_series;
    /** @brief The maximum allowed width update VarianceLimitingEnvelopeGrouper */
    Real m_max_width_change;
    /** @brief The size of each bucket for BucketingEnvelopeMerger */
    size_t m_bucket_size;
    /** @brief Type of search method used (TREE_ENVELOPE or VL_ENVELOPE) */
    IndexType m_type;
};

/** @brief Parameters for indexes that group envelopes into trees */
struct TreeEnvelopeIndexProperties : virtual EnvelopeIndexProperties, virtual SaxIndexProperties {
    /** @brief Params for envelope grouping */
    EnvelopeGroupingProperties m_env_grouping_params;

    IndexType get_type() const override { return m_env_grouping_params.m_type; }

    /**
     * @brief Constructor
     * @param segmentation_params Segmentation parameters
     * @param merger_params Merger parameters
     * @param pos_per_env Size of the starting position groups
     * @param sax_params SAX parameters
     * @param bucket_size The size of each bucket in the tree
     */
    TreeEnvelopeIndexProperties(SegmentationProperties segmentation_params, MergerProperties merger_params,
                                uint pos_per_env, SaxProperties sax_params,
                                EnvelopeGroupingProperties env_grouping_params)
        : PaaIndexProperties(segmentation_params, merger_params),
          EnvelopeIndexProperties(segmentation_params, merger_params, pos_per_env),
          SaxIndexProperties(segmentation_params, merger_params, sax_params),
          m_env_grouping_params(env_grouping_params) {}
};

#endif  // INDEX_INDEXPARAMS_HPP

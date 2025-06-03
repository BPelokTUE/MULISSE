#ifndef INDEX_INDEXPARAMS_HPP
#define INDEX_INDEXPARAMS_HPP

#include "Enums/ChannelSegmentationStrategyType.hpp"
#include "Enums/EntryMergerType.hpp"
#include "Enums/EnvelopeScoresTypes.hpp"
#include "Enums/LengthGroupSegmentationStrategyType.hpp"
#include "Enums/SaxBreakpointStrategyType.hpp"
#include "Enums/SearchMethodType.hpp"
#include "Enums/SegmentationStrategyType.hpp"
#include "Enums/iSaxSplitStrategyType.hpp"
#include "Index/Segmentation/ChannelSegmentationStrategy/ScoreBased/ScoreBasedChSSParams.hpp"
#include "Util/Types/Numbers.hpp"

/** @brief Interface for index parameters */
struct IIndexParams {
    virtual ~IIndexParams() = default;

    /**
     * @brief Get the type of the index
     * @return The type of the index
     */
    virtual SearchMethodType get_type() const = 0;
};

struct SegmentationParams {
    /** @brief Number of segments to use */
    SaxSegIndT m_num_segments;
    /** @brief Type of strategy to use for length group segmentation */
    LengthGroupSegmentationStrategyType m_lg_strategy_type;
    /** @brief Type of strategy to use for channel segmentation */
    ChannelSegmentationStrategyType m_ch_strategy_type;
    /** @brief Type of strategy to use for segmentation */
    SegmentationStrategyType m_strategy_type;
    /** @brief File containing the proportions of segments  */
    const str m_ch_num_seg_props_file;
    /** @brief Parameters of ScoreBasedChSegmentationStrategy */
    const ScoreBasedChSSParams *m_score_based_chss_params;
};

struct SaxParams {
    /** @brief Number of symbols to use for the SAX representations */
    SaxNumBitsT m_num_bits;
    /** @brief Strategy for getting the breakpoints of the symbol intervals */
    SaxBreakpointStrategyType m_breakpoint_strategy_type;
    /** @brief Only used for FixedBreakpointStrategy: path to the plain text file to load the fixed breakpoints from */
    str m_breakpoints_file;
};

struct MergerParams {
    /** @brief Type of entry merger to use */
    EntryMergerType m_entry_merger_type;
    /** @brief SAX parameters for SAX-based mergers, nullptr for non-SAX-based mergers */
    const SaxParams *m_merger_sax_params;
};

struct PaaIndexParams : virtual IIndexParams {
    /** @brief segmentation parameters */
    SegmentationParams m_segmentation_params;
    /** @brief merger parameters */
    MergerParams m_merger_params;

    /**
     * @brief Constructor
     * @param segmentation_params Segmentation parameters
     * @param merger_params Merger parameters
     */
    PaaIndexParams(SegmentationParams segmentation_params, MergerParams merger_params)
        : m_segmentation_params(segmentation_params), m_merger_params(merger_params) {}
};

/** @brief Parameters for indexes that use SAX */
struct SaxIndexParams : virtual PaaIndexParams {
    /** @brief SAX parameters */
    SaxParams m_sax_params;

    /**
     * @brief Constructor
     * @param segmentation_params Segmentation parameters
     * @param merger_params Merger parameters
     * @param sax_params SAX parameters
     */
    SaxIndexParams(SegmentationParams segmentation_params, MergerParams merger_params, SaxParams sax_params)
        : PaaIndexParams(segmentation_params, merger_params), m_sax_params(sax_params) {}
};

/** @brief Parameters for indexes that use envelopes */
struct EnvelopeIndexParams : virtual PaaIndexParams {
    SearchMethodType get_type() const override { return ENVELOPE; }

    /** @brief Size of the starting position groups */
    uint m_pos_per_env;

    /**
     * @brief Constructor
     * @param segmentation_params Segmentation parameters
     * @param merger_params Merger parameters
     * @param pos_per_env Size of the starting position groups
     */
    EnvelopeIndexParams(SegmentationParams segmentation_params, MergerParams merger_params, uint pos_per_env)
        : PaaIndexParams(segmentation_params, merger_params), m_pos_per_env(pos_per_env) {}
};

/** @brief Parameters for SAX Envelope indexes */
struct SaxEnvelopeIndexParams : virtual EnvelopeIndexParams, virtual SaxIndexParams {
    SearchMethodType get_type() const override { return SAX_ENVELOPE; }

    /**
     * @brief Constructor
     * @param segmentation_params Segmentation parameters
     * @param merger_params Merger parameters
     * @param pos_per_env Size of the starting position groups
     * @param sax_params iSAX parameters
     */
    SaxEnvelopeIndexParams(SegmentationParams segmentation_params, MergerParams merger_params, uint pos_per_env,
                           SaxParams sax_params)
        : PaaIndexParams(segmentation_params, merger_params),
          EnvelopeIndexParams(segmentation_params, merger_params, pos_per_env),
          SaxIndexParams(segmentation_params, merger_params, sax_params) {}
};

struct iSaxTrieParams {
    /** @brief Whether to merge entries in the leaves of the tree */
    bool m_merge_in_leaves;
    /** @brief Only used for EntropyMaximizingSplitStrategy: whether to select the segment with the min number of bits
     * in case of a tie */
    bool m_min_num_bits_on_tie;
    /** @brief Maximum number of bits per segment */
    SaxNumBitsT m_num_bits_limit;
    /** @brief Strategy for choosing the index to split on */
    iSaxSplitStrategyType m_split_strategy_type;
    /** @brief Maximum number of entries in a leaf */
    size_t m_leaf_capacity;
};

/**
 * @brief Parameters for iSAX indexes
 * iSAX indexes split subsequences into segments and encode them using iSAX words
 */
struct iSaxIndexParams : virtual PaaIndexParams, virtual SaxIndexParams {
    /** @brief iSAX trie parameters */
    iSaxTrieParams m_isax_trie_params;

    SearchMethodType get_type() const override { return ISAX; }

    /**
     * @brief Constructor
     * @param segmentation_params Segmentation parameters
     * @param merger_params Merger parameters
     * @param sax_params SAX parameters
     * @param isax_trie_params iSAX trie parameters
     */
    iSaxIndexParams(SegmentationParams segmentation_params, MergerParams merger_params, SaxParams sax_params,
                    iSaxTrieParams isax_trie_params)
        : PaaIndexParams(segmentation_params, merger_params),
          SaxIndexParams(segmentation_params, merger_params, sax_params),
          m_isax_trie_params(isax_trie_params) {}
};

/** @brief Parameters for an iSaxIndex<Envelope> */
struct iSaxEnvelopeIndexParams : virtual EnvelopeIndexParams, virtual iSaxIndexParams {
    SearchMethodType get_type() const override { return ISAX_ENVELOPE; }

    /**
     * @brief Constructor
     * @param segmentation_params Segmentation parameters
     * @param merger_params Merger parameters
     * @param pos_per_env Size of the starting position groups
     * @param enveloping_params Envelope parameters
     * @param sax_params SAX parameters
     * @param isax_trie_params iSAX trie parameters
     */
    iSaxEnvelopeIndexParams(SegmentationParams segmentation_params, MergerParams merger_params, uint pos_per_env,
                            SaxParams sax_params, iSaxTrieParams isax_trie_params)
        : PaaIndexParams(segmentation_params, merger_params),
          EnvelopeIndexParams(segmentation_params, merger_params, pos_per_env),
          SaxIndexParams(segmentation_params, merger_params, sax_params),
          iSaxIndexParams(segmentation_params, merger_params, sax_params, isax_trie_params) {}
};

struct EnvelopeGroupingParams {
    /** @brief The maximum allowed width update VarianceLimitingEnvelopeGrouper */
    Real m_max_width_change;
    /** @brief The size of each bucket for BucketingEnvelopeMerger */
    size_t m_bucket_size;
    /** @brief Type of search method used (TREE_ENVELOPE or VL_ENVELOPE) */
    SearchMethodType m_type;
};

/** @brief Parameters for TreeEnvelopeIndex */
struct TreeEnvelopeIndexParams : virtual EnvelopeIndexParams, virtual SaxIndexParams {
    /** @brief Params for envelope grouping */
    EnvelopeGroupingParams m_env_grouping_params;

    SearchMethodType get_type() const override { return m_env_grouping_params.m_type; }

    /**
     * @brief Constructor
     * @param segmentation_params Segmentation parameters
     * @param merger_params Merger parameters
     * @param pos_per_env Size of the starting position groups
     * @param sax_params SAX parameters
     * @param bucket_size The size of each bucket in the tree
     */
    TreeEnvelopeIndexParams(SegmentationParams segmentation_params, MergerParams merger_params, uint pos_per_env,
                            SaxParams sax_params, EnvelopeGroupingParams env_grouping_params)
        : PaaIndexParams(segmentation_params, merger_params),
          EnvelopeIndexParams(segmentation_params, merger_params, pos_per_env),
          SaxIndexParams(segmentation_params, merger_params, sax_params),
          m_env_grouping_params(env_grouping_params) {}
};

#endif  // INDEX_INDEXPARAMS_HPP

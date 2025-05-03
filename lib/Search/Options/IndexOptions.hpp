#ifndef INDEX_OPTIONS_HPP
#define INDEX_OPTIONS_HPP

#include "Search/Options/SearchMethodType.hpp"
#include "Search/iSax/iSaxSplitStrategy.hpp"
#include "Summarization/SegmentationStrategy.hpp"
#include "Summarization/iSaxBreakpointStrategy.hpp"
#include "Util/utilities.hpp"
#include "Util/typedefs.hpp"

/** @brief Enum for IEntryInserter implementations */
enum EntryInserterType { TOP_DOWN, ISAX_PARALLEL };

DEFINE_ENUM_CONSTS_NO_EXTRA(EntryInserterType, ENTRY_INSERTER_TYPE, false);

/** @brief Interface for index parameters */
struct IIndexParams {
    virtual ~IIndexParams() = default;

    /**
     * @brief Get the type of the index
     * @return The type of the index
     */
    virtual SearchMethodType get_type() const = 0;
};
struct PaaIndexParams : virtual IIndexParams {
    /** @brief Type of strategy to use for segmentation */
    SegmentationStrategyType m_segmentation_strategy_type;
    /** @brief Whether to use different segmentation strategy per length-group */
    bool m_per_lg_segmentation;
    /** @brief Number of segments to use */
    SaxSegIndT m_num_segments;

    /**
     * @brief Constructor
     * @param segmentation_strategy_type Type of strategy to use for segmentation
     * @param per_lg_segmentation Whether to use different segmentation strategy per length-group
     * @param num_segments Number of segments to use
     */
    PaaIndexParams(SegmentationStrategyType segmentation_strategy_type, bool per_lg_segmentation,
                   SaxSegIndT num_segments)
        : m_segmentation_strategy_type(segmentation_strategy_type),
          m_per_lg_segmentation(per_lg_segmentation),
          m_num_segments(num_segments) {}
};

/** @brief Parameters for indexes that use envelopes */
struct EnvelopeIndexParams : virtual PaaIndexParams {
    SearchMethodType get_type() const override { return ENVELOPE; }

    /** @brief Size of the starting position groups */
    uint m_pos_per_env;

    /**
     * @brief Constructor
     * @param segmentation_strategy_type Type of strategy to use for segmentation
     * @param per_lg_segmentation Whether to use different segmentation strategy per length-group
     * @param num_segments Number of segments to use
     * @param pos_per_env Size of the starting position groups
     */
    EnvelopeIndexParams(SegmentationStrategyType segmentation_strategy_type, bool per_lg_segmentation,
                        SaxSegIndT num_segments, uint pos_per_env)
        : PaaIndexParams(segmentation_strategy_type, per_lg_segmentation, num_segments), m_pos_per_env(pos_per_env) {}
};

/** @brief Parameters for indexes that use SAX */
struct SaxIndexParams : virtual PaaIndexParams {
    /** @brief Number of symbols to use for the SAX representations */
    SaxNumBitsT m_num_bits;
    /** @brief Strategy for getting the breakpoints of the symbol intervals */
    iSaxBreakpointStrategyType m_breakpoint_strategy_type;
    /** @brief Only used for EntropyMaximizingStrategy: whether to select the segment with the min number of bits in
     * case of a tie */
    bool m_min_num_bits_on_tie;
    /** @brief Only used for FixedBreakpointStrategy: path to the plain text file to load the fixed breakpoints from */
    str m_breakpoints_file;

    /**
     * @brief Constructor
     * @param segmentation_strategy_type Type of strategy to use for segmentation
     * @param per_lg_segmentation Whether to use different segmentation strategy per length-group
     * @param num_segments Number of segments to use
     * @param num_bits Number of bits to use for the SAX representations
     * @param breakpoint_strategy_type Strategy for getting the breakpoints of the symbol intervals
     * @param min_num_bits_on_tie Only used for EntropyMaximizingStrategy: whether to select the segment with the min
     * @param breakpoints_file Only used for FixedBreakpointStrategy: path to the plain text file to load the fixed
     * breakpoints from
     */
    SaxIndexParams(SegmentationStrategyType segmentation_strategy_type, bool per_lg_segmentation,
                   SaxSegIndT num_segments, SaxNumBitsT num_bits, iSaxBreakpointStrategyType breakpoint_strategy_type,
                   bool min_num_bits_on_tie, const str &breakpoints_file)
        : PaaIndexParams(segmentation_strategy_type, per_lg_segmentation, num_segments),
          m_num_bits(num_bits),
          m_breakpoint_strategy_type(breakpoint_strategy_type),
          m_min_num_bits_on_tie(min_num_bits_on_tie),
          m_breakpoints_file(breakpoints_file) {}
};

/** @brief Parameters for SAX Envelope indexes */
struct SaxEnvelopeIndexParams : virtual EnvelopeIndexParams, virtual SaxIndexParams {
    SearchMethodType get_type() const override { return SAX_ENVELOPE; }

    /**
     * @brief Constructor
     * @param segmentation_strategy_type Type of strategy to use for segmentation
     * @param per_lg_segmentation Whether to use different segmentation strategy per length-group
     * @param num_segments Number of segments to use
     * @param pos_per_env Size of the starting position groups
     * @param num_bits Number of bits to use for the SAX representations
     * @param breakpoint_strategy_type Strategy for getting the breakpoints of the symbol intervals
     * @param min_num_bits_on_tie Only used for EntropyMaximizingStrategy: whether to select the segment with the min
     * number of bits in case of a tie
     * @param breakpoints_file Only used for FixedBreakpointStrategy: path to the plain text file to load the fixed
     * breakpoints from
     */
    SaxEnvelopeIndexParams(SegmentationStrategyType segmentation_strategy_type, bool per_lg_segmentation,
                           SaxSegIndT num_segments, uint pos_per_env, SaxNumBitsT num_bits,
                           iSaxBreakpointStrategyType breakpoint_strategy_type, bool min_num_bits_on_tie,
                           const str &breakpoints_file)
        : PaaIndexParams(segmentation_strategy_type, per_lg_segmentation, num_segments),
          EnvelopeIndexParams(segmentation_strategy_type, per_lg_segmentation, num_segments, pos_per_env),
          SaxIndexParams(segmentation_strategy_type, per_lg_segmentation, num_segments, num_bits,
                         breakpoint_strategy_type, min_num_bits_on_tie, breakpoints_file) {}
};

/**
 * @brief Parameters for iSAX indexes
 * iSAX indexes split subsequences into segments and encode them using iSAX words
 */
struct iSaxIndexParams : virtual PaaIndexParams, virtual SaxIndexParams {
    SearchMethodType get_type() const override { return ISAX; }

    /** @brief Maximum number of entries in a leaf */
    size_t m_leaf_capacity;
    /** @brief Strategy for choosing the index to split on */
    iSaxSplitStrategyType m_split_strategy_type;
    /** @brief Maximum number of bits per segment */
    SaxNumBitsT m_num_bits_limit;

    /**
     * @brief Constructor
     * @param segmentation_strategy_type Type of strategy to use for segmentation
     * @param per_lg_segmentation Whether to use different segmentation strategy per length-group
     * @param num_segments Number of segments to use
     * @param first_layer_num_bits Number of bits to use for symbols in the first layer of the index
     * @param leaf_capacity Maximum number of entries in a leaf
     * @param breakpoint_strategy_type Strategy for getting the breakpoints of the symbol intervals
     * @param split_strategy_type Type of strategy for choosing the index to split on
     * @param num_bits_limit Maximum number of bits per segment
     * @param min_num_bits_on_tie Only used for EntropyMaximizingStrategy: whether to select the segment with the min
     * number of bits in case of a tie
     * @param breakpoints_file Only used for FixedBreakpointStrategy: path to the plain text file to load the fixed
     * breakpoints
     */
    iSaxIndexParams(SegmentationStrategyType segmentation_strategy_type, bool per_lg_segmentation,
                    SaxSegIndT num_segments, SaxNumBitsT first_layer_num_bits, size_t leaf_capacity,
                    iSaxBreakpointStrategyType breakpoint_strategy_type, iSaxSplitStrategyType split_strategy_type,
                    SaxNumBitsT num_bits_limit, bool min_num_bits_on_tie, const str &breakpoints_file)
        : PaaIndexParams(segmentation_strategy_type, per_lg_segmentation, num_segments),
          SaxIndexParams(segmentation_strategy_type, per_lg_segmentation, num_segments, first_layer_num_bits,
                         breakpoint_strategy_type, min_num_bits_on_tie, breakpoints_file),
          m_leaf_capacity(leaf_capacity),
          m_split_strategy_type(split_strategy_type),
          m_num_bits_limit(num_bits_limit) {}
};

/** @brief Parameters for an iSaxIndex<Envelope> */
struct iSaxEnvelopeIndexParams : virtual EnvelopeIndexParams, virtual iSaxIndexParams {
    SearchMethodType get_type() const override { return ISAX_ENVELOPE; }

    /**
     * @brief Constructor
     *
     * @param segmentation_strategy_type Type of strategy to use for segmentation
     * @param per_lg_segmentation Whether to use different segmentation strategy per length-group
     * @param num_segments Number of segments to use
     * @param pos_per_env Size of the starting position groups
     * @param first_layer_num_bits Number of bit to use for symbols in the first layer of the index
     * @param leaf_capacity Maximum number of entries in a leaf
     * @param breakpoint_strategy_type Strategy for getting the breakpoints of the symbol intervals
     * @param split_strategy_type Type of strategy for choosing the index to split on
     * @param num_bits_limit Maximum number of bits per segment
     * @param min_num_bits_on_tie Only used for EntropyMaximizingStrategy: whether to select the segment with the min
     * number of bits in case of a tie
     * @param breakpoints_file Only used for FixedBreakpointStrategy: path to the plain text file to load the fixed
     * breakpoints from
     */
    iSaxEnvelopeIndexParams(SegmentationStrategyType segmentation_strategy_type, bool per_lg_segmentation,
                            SaxSegIndT num_segments, uint pos_per_env, SaxNumBitsT first_layer_num_bits,
                            size_t leaf_capacity, iSaxBreakpointStrategyType breakpoint_strategy_type,
                            iSaxSplitStrategyType split_strategy_type, SaxNumBitsT num_bits_limit,
                            bool min_num_bits_on_tie, const str &breakpoints_file)
        : iSaxIndexParams(segmentation_strategy_type, per_lg_segmentation, num_segments, first_layer_num_bits,
                          leaf_capacity, breakpoint_strategy_type, split_strategy_type, num_bits_limit,
                          min_num_bits_on_tie, breakpoints_file),
          PaaIndexParams(segmentation_strategy_type, per_lg_segmentation, num_segments),
          EnvelopeIndexParams(segmentation_strategy_type, per_lg_segmentation, num_segments, pos_per_env),
          SaxIndexParams(segmentation_strategy_type, per_lg_segmentation, num_segments, first_layer_num_bits,
                         breakpoint_strategy_type, min_num_bits_on_tie, breakpoints_file) {}
};

/** @brief Parameters for TreeEnvelopeIndex */
struct TreeEnvelopeIndexParams : virtual EnvelopeIndexParams, virtual SaxIndexParams {
    SearchMethodType get_type() const override { return TREE_ENVELOPE; }

    /** @brief The size of each bucket in the tree */
    size_t m_bucket_size;

    /**
     * @brief Constructor
     *
     * @param segmentation_strategy_type Type of strategy to use for segmentation
     * @param per_lg_segmentation Whether to use different segmentation strategy per length-group
     * @param num_segments Number of segments to use
     * @param pos_per_env Size of the starting position groups
     * @param inv_sax_num_bits Number of bits to use for the invSAX representations
     * @param bucket_size Size of buckets
     * @param breakpoint_strategy_type Strategy for getting the breakpoints of the symbol intervals
     * @param min_num_bits_on_tie Only used for EntropyMaximizingStrategy: whether to select the segment with the min
     * number of bits in case of a tie
     * @param breakpoints_file Only used for FixedBreakpointStrategy: path to the plain text file to load the fixed
     * breakpoints from
     */
    TreeEnvelopeIndexParams(SegmentationStrategyType segmentation_strategy_type, bool per_lg_segmentation,
                            SaxSegIndT num_segments, uint pos_per_env, SaxNumBitsT inv_sax_num_bits, size_t bucket_size,
                            iSaxBreakpointStrategyType breakpoint_strategy_type, bool min_num_bits_on_tie,
                            const str &breakpoints_file)
        : PaaIndexParams(segmentation_strategy_type, per_lg_segmentation, num_segments),
          EnvelopeIndexParams(segmentation_strategy_type, per_lg_segmentation, num_segments, pos_per_env),
          SaxIndexParams(segmentation_strategy_type, per_lg_segmentation, num_segments, inv_sax_num_bits,
                         breakpoint_strategy_type, min_num_bits_on_tie, breakpoints_file),
          m_bucket_size(bucket_size) {}
};

/** @brief Enumeration type for the cereal archives */
enum ArchiveType { BINARY, JSON, NONE };

DEFINE_ENUM_CONSTS_NO_EXTRA(ArchiveType, ARCHIVE_TYPE, false);

/**
 * @brief Get the extension for a given archive type
 * @param ar_type The archive type
 * @return The extension for the archive type
 */
inline str get_archive_extension(ArchiveType ar_type) {
    switch (ar_type) {
        case BINARY:
            return ".bin";
        case JSON:
            return ".json";
        default:
            return "";
    }
}

/**
 * @brief Add the extension for a given archive type to a file name, if not already present
 * @param file_name The file name
 * @return The file name with the extension added, if not already present
 */
inline str add_archive_extension(const str &file_name, ArchiveType ar_type) {
    auto [base, extension] = get_file_base_and_extension(file_name);
    return base + (extension.empty() ? get_archive_extension(ar_type) : extension);
}

/** @brief Options for creating an index */
struct IndexOptions {
    /** @brief Whether to Z-normalize the subsequences */
    bool m_normalized;
    /** @brief Whether to adapt the index properties to the dataset entries */
    bool m_adapt;
    /** @brief Whether to use length groups */
    bool m_use_length_groups;
    /** @brief The type of the index method to use */
    SearchMethodType m_index_method;
    /** @brief Format to save the index in */
    ArchiveType m_index_format;
    /** @brief Type of inserter to use */
    EntryInserterType m_inserter_type;
    /** @brief Number of channels of each series */
    MtsNumChannelsT m_num_channels;
    /** @brief Minimum accepted query length */
    uint m_l_min;
    /** @brief Maximum accepted query length */
    uint m_l_max;
    /** @brief Length time series in the dataset */
    uint m_series_len;
    /** @brief Lengths per group */
    uint m_l_per_group;
    /** @brief Unique pointer to the index parameters */
    std::unique_ptr<IIndexParams> m_index_params;
};

#endif  // INDEX_OPTIONS_HPP

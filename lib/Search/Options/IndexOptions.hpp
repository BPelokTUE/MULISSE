#ifndef INDEX_OPTIONS_HPP
#define INDEX_OPTIONS_HPP

#include "Search/Options/SearchMethodType.hpp"
#include "Search/iSax/iSaxSplitStrategy.hpp"
#include "Summarization/SegmentationStrategy.hpp"
#include "Summarization/LengthGroupSegmentationStrategy.hpp"
#include "Summarization/iSaxBreakpointStrategy.hpp"
#include "Summarization/EntryMerger.hpp"
#include "Util/utilities.hpp"
#include "Util/typedefs.hpp"

/** @brief Enum for IEntryInserter implementations */
enum EntryInserterType { TOP_DOWN, PARALLEL };

DEFINE_ENUM_CONSTS(EntryInserterType, ENTRY_INSERTER_TYPE, false,
                   (umap<str, EntryInserterType>{{"isax_parallel", PARALLEL}}));

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
    /** @brief Type of strategy to use for segmentation */
    SegmentationStrategyType m_strategy_type;
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
    /** @brief Only used for EntropyMaximizingStrategy: whether to select the segment with the min number of bits in
     * case of a tie */
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

/** @brief Parameters for TreeEnvelopeIndex */
struct TreeEnvelopeIndexParams : virtual EnvelopeIndexParams, virtual SaxIndexParams {
    SearchMethodType get_type() const override { return TREE_ENVELOPE; }

    /** @brief The size of each bucket in the tree */
    size_t m_bucket_size;

    /**
     * @brief Constructor
     * @param segmentation_params Segmentation parameters
     * @param merger_params Merger parameters
     * @param pos_per_env Size of the starting position groups
     * @param sax_params SAX parameters
     * @param bucket_size The size of each bucket in the tree
     */
    TreeEnvelopeIndexParams(SegmentationParams segmentation_params, MergerParams merger_params, uint pos_per_env,
                            SaxParams sax_params, size_t bucket_size)
        : PaaIndexParams(segmentation_params, merger_params),
          EnvelopeIndexParams(segmentation_params, merger_params, pos_per_env),
          SaxIndexParams(segmentation_params, merger_params, sax_params),
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
    /** @brief Number of channels of each series */
    MtsNumChannelsT m_num_channels;
    /** @brief The type of the index method to use */
    SearchMethodType m_index_method;
    /** @brief Format to save the index in */
    ArchiveType m_index_format;
    /** @brief Type of inserter to use */
    EntryInserterType m_inserter_type;
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

#ifndef INDEX_OPTIONS_HPP
#define INDEX_OPTIONS_HPP

#include "Search/Options/SearchMethodType.hpp"
#include "Search/iSax/iSaxSplitStrategy.hpp"
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
    /** @brief Length of the segments */
    uint segment_len;

    PaaIndexParams(uint segment_len) : segment_len(segment_len) {}
};

/** @brief Parameters for indexes that use envelopes */
struct EnvelopeIndexParams : virtual PaaIndexParams {
    SearchMethodType get_type() const override { return ENVELOPE; }

    /** @brief Size of the starting position groups */
    uint pos_per_env;

    /**
     * @brief Constructor
     * @param pos_per_env Size of the starting position groups
     * @param segment_len Length of the segments
     */
    EnvelopeIndexParams(uint pos_per_env, uint segment_len) : PaaIndexParams(segment_len), pos_per_env(pos_per_env) {}
};

/** @brief Parameters for indexes that use SAX */
struct SaxIndexParams : virtual PaaIndexParams {
    /** @brief Number of symbols to use for the SAX representations */
    SaxNumBitsT num_bits;
    /** @brief Strategy for getting the breakpoints of the symbol intervals */
    iSaxBreakpointStrategyType breakpoint_strategy_type;
    /** @brief Only used for EntropyMaximizingStrategy: whether to select the segment with the min number of bits in
     * case of a tie */
    bool min_num_bits_on_tie;
    /** @brief Only used for FixedBreakpointStrategy: path to the plain text file to load the fixed breakpoints from */
    str breakpoints_file;

    /**
     * @brief Constructor
     * @param segment_len Length of the segments
     * @param num_bits Number of symbols to use for the SAX representations
     * @param breakpoint_strategy_type Strategy for getting the breakpoints of the symbol intervals
     * @param min_num_bits_on_tie Only used for EntropyMaximizingStrategy: whether to select the segment with the min
     * @param breakpoints_file Only used for FixedBreakpointStrategy: path to the plain text file to load the fixed
     * breakpoints from
     */
    SaxIndexParams(uint segment_len, SaxNumBitsT num_bits, iSaxBreakpointStrategyType breakpoint_strategy_type,
                   bool min_num_bits_on_tie, const str &breakpoints_file)
        : PaaIndexParams(segment_len),
          num_bits(num_bits),
          breakpoint_strategy_type(breakpoint_strategy_type),
          min_num_bits_on_tie(min_num_bits_on_tie),
          breakpoints_file(breakpoints_file) {}
};

/** @brief Parameters for SAX Envelope indexes */
struct SaxEnvelopeIndexParams : virtual EnvelopeIndexParams, virtual SaxIndexParams {
    SearchMethodType get_type() const override { return SAX_ENVELOPE; }

    /**
     * @brief Constructor
     * @param pos_per_env Size of the starting position groups
     * @param segment_len Length of the segments
     * @param num_bits Number of symbols to use for the SAX representations
     * @param breakpoint_strategy_type Strategy for getting the breakpoints of the symbol intervals
     * @param min_num_bits_on_tie Only used for EntropyMaximizingStrategy: whether to select the segment with the min
     * number of bits in case of a tie
     * @param breakpoints_file Only used for FixedBreakpointStrategy: path to the plain text file to load the fixed
     * breakpoints from
     */
    SaxEnvelopeIndexParams(uint pos_per_env, uint segment_len, SaxNumBitsT num_bits,
                           iSaxBreakpointStrategyType breakpoint_strategy_type, bool min_num_bits_on_tie,
                           const str &breakpoints_file)
        : PaaIndexParams(segment_len),
          EnvelopeIndexParams(pos_per_env, segment_len),
          SaxIndexParams(segment_len, num_bits, breakpoint_strategy_type, min_num_bits_on_tie, breakpoints_file) {}
};

/**
 * @brief Parameters for iSAX indexes
 * iSAX indexes split subsequences into segments and encode them using iSAX words
 */
struct iSaxIndexParams : virtual PaaIndexParams, virtual SaxIndexParams {
    SearchMethodType get_type() const override { return ISAX; }

    /** @brief Maximum number of entries in a leaf */
    size_t leaf_capacity;
    /** @brief Strategy for choosing the index to split on */
    iSaxSplitStrategyType split_strategy_type;
    /** @brief Maximum number of bits per segment */
    SaxNumBitsT num_bits_limit;

    /**
     * @brief Constructor
     * @param segment_len Length of the segments
     * @param first_layer_num_bits Number of symbols to use in the first layer of the index
     * @param leaf_capacity Maximum number of entries in a leaf
     * @param breakpoint_strategy_type Strategy for getting the breakpoints of the symbol intervals
     * @param split_strategy_type Strategy for choosing the index to split on
     * @param num_bits_limit Maximum number of bits per segment
     * @param min_num_bits_on_tie Only used for EntropyMaximizingStrategy: whether to select the segment with the min
     * number of bits in case of a tie
     * @param breakpoints_file Only used for FixedBreakpointStrategy: path to the plain text file to load the fixed
     * breakpoints
     */
    iSaxIndexParams(uint segment_len, SaxNumBitsT first_layer_num_bits, size_t leaf_capacity,
                    iSaxBreakpointStrategyType breakpoint_strategy_type, iSaxSplitStrategyType split_strategy_type,
                    SaxNumBitsT num_bits_limit, bool min_num_bits_on_tie, const str &breakpoints_file)
        : PaaIndexParams(segment_len),
          SaxIndexParams(segment_len, first_layer_num_bits, breakpoint_strategy_type, min_num_bits_on_tie,
                         breakpoints_file),
          leaf_capacity(leaf_capacity),
          split_strategy_type(split_strategy_type),
          num_bits_limit(num_bits_limit) {}
};

/** @brief Parameters for an iSAX envelope (ULISSE) index */
struct iSaxEnvelopeIndexParams : virtual EnvelopeIndexParams, virtual iSaxIndexParams {
    SearchMethodType get_type() const override { return ISAX_ENVELOPE; }

    /**
     * @brief Constructor
     * @param pos_per_env Size of the starting position groups
     * @param segment_len Length of the segments
     * @param first_layer_num_bits Number of symbols to use in the first layer of the index
     * @param leaf_capacity Maximum number of entries in a leaf
     * @param breakpoint_strategy_type Strategy for getting the breakpoints of the symbol intervals
     * @param split_strategy_type Strategy for choosing the index to split on
     * @param num_bits_limit Maximum number of bits per segment
     * @param min_num_bits_on_tie Only used for EntropyMaximizingStrategy: whether to select the segment with the min
     * number of bits in case of a tie
     * @param breakpoints_file Only used for FixedBreakpointStrategy: path to the plain text file to load the fixed
     * breakpoints from
     */
    iSaxEnvelopeIndexParams(uint pos_per_env, uint segment_len, SaxNumBitsT first_layer_num_bits, size_t leaf_capacity,
                            iSaxBreakpointStrategyType breakpoint_strategy_type,
                            iSaxSplitStrategyType split_strategy_type, SaxNumBitsT num_bits_limit,
                            bool min_num_bits_on_tie, const str &breakpoints_file)
        : iSaxIndexParams(segment_len, first_layer_num_bits, leaf_capacity, breakpoint_strategy_type,
                          split_strategy_type, num_bits_limit, min_num_bits_on_tie, breakpoints_file),
          EnvelopeIndexParams(pos_per_env, segment_len),
          PaaIndexParams(segment_len),
          SaxIndexParams(segment_len, first_layer_num_bits, breakpoint_strategy_type, min_num_bits_on_tie,
                         breakpoints_file) {}
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
        case NONE:
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
    bool normalized;
    /** @brief Whether to adapt the index properties to the dataset entries */
    bool adapt;
    /** @brief The type of the index method to use */
    SearchMethodType index_method;
    /** @brief Format to save the index in */
    ArchiveType index_format;
    /** @brief Type of inserter to use */
    EntryInserterType inserter_type;
    /** @brief Number of channels of each series */
    MtsNumChannelsT num_channels;
    /** @brief Minimum accepted query length */
    uint l_min;
    /** @brief Maximum accepted query length */
    uint l_max;
    /** @brief Length time series in the dataset */
    uint series_len;
    /** @brief Lengths per group */
    uint l_per_group;
    /** @brief Unique pointer to the index parameters */
    std::unique_ptr<IIndexParams> index_params;

    /** @brief Get the number of lengths per length group */
    uint get_num_len_groups() const {
        return l_per_group > 0 ? ((l_max - l_min + 1) + l_per_group - 1) / l_per_group : 1;
    }
};

#endif  // INDEX_OPTIONS_HPP

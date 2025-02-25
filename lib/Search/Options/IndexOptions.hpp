#ifndef INDEX_OPTIONS_HPP
#define INDEX_OPTIONS_HPP

#include "Summarization/iSaxBreakpointStrategy.hpp"
#include "Search/Options/SearchMethodType.hpp"
#include "Search/iSax/iSaxSplitStrategy.hpp"
#include "Util/utilities.hpp"
#include "Util/typedefs.hpp"

/** @brief Interface for index parameters */
struct IIndexParams {
    virtual ~IIndexParams() = default;

    /**
     * @brief Get the type of the index
     * @return The type of the index
     */
    virtual SearchMethodType get_type() const = 0;
};

/**
 * @brief Parameters for envelope indexes
 *
 * Envelope indexes group together subsequences by their starting position into Envelope objects
 * */
struct EnvelopeIndexParams : virtual IIndexParams {
    /** @brief Size of the starting position groups */
    uint pos_per_env;
};

/**
 * @brief Parameters for iSAX indexes
 *
 * iSAX indexes split subsequences into segments and encode them using iSAX words
 */
struct iSaxIndexParams : virtual IIndexParams {
    SearchMethodType get_type() const override { return ISAX; }

    /** @brief Length of the segments */
    uint segment_len;
    /** @brief Number of symbols to use in the first layer of the index */
    SaxNumBitsT first_layer_num_bits;
    /** @brief Maximum number of entries in a leaf */
    size_t leaf_capacity;
    /** @brief Strategy for getting the breakpoints of the symbol intervals */
    iSaxBreakpointStrategyType breakpoint_strategy_type;
    /** @brief Strategy for choosing the index to split on */
    iSaxSplitStrategyType split_strategy_type;
    /** @brief Maximum number of bits per segment */
    SaxNumBitsT num_bits_limit;
    /** @brief Only used for EntropyMaximizingStrategy: whether to select the segment with the min number of bits in
     * case of a tie */
    bool min_num_bits_on_tie;

    /**
     * @brief Constructor
     * @param segment_len Length of the segments
     * @param first_layer_num_bits Number of symbols to use in the first layer of the index
     * @param leaf_capacity Maximum number of entries in a leaf
     * @param breakpoint_strategy_type Strategy for getting the breakpoints of the symbol intervals
     * @param split_strategy_type Strategy for choosing the index to split on
     * @param num_bits_limit Maximum number of bits per segment
     * @param min_num_bits_on_tie Only used for EntropyMaximizingStrategy: whether to select the segment with the min
     *        number of bits in case of a tie
     */
    iSaxIndexParams(uint segment_len, SaxNumBitsT first_layer_num_bits, size_t leaf_capacity,
                    iSaxBreakpointStrategyType breakpoint_strategy_type, iSaxSplitStrategyType split_strategy_type,
                    SaxNumBitsT num_bits_limit, bool min_num_bits_on_tie)
        : segment_len(segment_len),
          first_layer_num_bits(first_layer_num_bits),
          leaf_capacity(leaf_capacity),
          breakpoint_strategy_type(breakpoint_strategy_type),
          split_strategy_type(split_strategy_type),
          num_bits_limit(num_bits_limit),
          min_num_bits_on_tie(min_num_bits_on_tie) {}
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
     *        number of bits in case of a tie
     */
    iSaxEnvelopeIndexParams(uint pos_per_env, uint segment_len, SaxNumBitsT first_layer_num_bits, size_t leaf_capacity,
                            iSaxBreakpointStrategyType breakpoint_strategy_type,
                            iSaxSplitStrategyType split_strategy_type, SaxNumBitsT num_bits_limit,
                            bool min_num_bits_on_tie)
        : iSaxIndexParams(segment_len, first_layer_num_bits, leaf_capacity, breakpoint_strategy_type,
                          split_strategy_type, num_bits_limit, min_num_bits_on_tie) {
        this->pos_per_env = pos_per_env;
    }
};

/** @brief Enumeration type for the cereal archives */
enum ArchiveType { BINARY, JSON, NONE };

DEFINE_ENUM_CONSTS_NO_EXTRA(ArchiveType, ARCHIVE_TYPE, false);

/** @brief Options for creating an index */
struct IndexOptions {
    /** @brief Format to save the index in */
    ArchiveType index_format;
    /** @brief Minimum accepted query length */
    uint l_min;
    /** @brief Maximum accepted query length */
    uint l_max;
    /** @brief Length time series in the dataset */
    uint series_len;
    /** @brief Number of channels of each series */
    MtsNumChannelsT num_channels;
    /** @brief Whether to Z-normalize the subsequences */
    bool normalized;
    /** @brief Unique pointer to the index parameters */
    std::unique_ptr<IIndexParams> index_params;
};

#endif  // INDEX_OPTIONS_HPP

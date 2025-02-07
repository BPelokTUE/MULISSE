#ifndef INDEX_OPTIONS_HPP
#define INDEX_OPTIONS_HPP

#include "Summarization/iSaxBreakpointStrategy.hpp"
#include "Search/Options/SearchMethodType.hpp"
#include "Search/iSax/iSaxSplitStrategy.hpp"
#include "Util/utilities.hpp"

/** @brief Interface for index parameters */
struct IIndexParams {
    virtual ~IIndexParams() = default;

    /**
     * @brief Get the type of the index
     *
     * @return The type of the index
     */
    virtual SearchMethodType get_type() const = 0;
};

/**
 * @brief Parameters for envelope indexes
 *
 * Envelope indexes group together subsequences by their starting position into Envelope objects
 * */
struct EnvelopeIndexParams : IIndexParams {
    /** @brief Size of the starting position groups */
    uint pos_per_env;
};

/**
 * @brief Parameters for iSAX indexes
 *
 * iSAX indexes split subsequences into segments and encode them using iSAX words
 */
struct iSaxIndexParams {
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
};

/** @brief Parameters for an iSAX envelope (ULISSE) index */
struct iSaxEnvelopeIndexParams : EnvelopeIndexParams, iSaxIndexParams {
    SearchMethodType get_type() const override { return ISAX_ENVELOPE; }

    /**
     * @brief Constructor
     *
     * @param pos_per_env Size of the starting position groups
     * @param segment_len Length of the segments
     * @param first_layer_num_bits Number of symbols to use in the first layer of the index
     * @param leaf_capacity Maximum number of entries in a leaf
     * @param breakpoint_strategy_type Strategy for getting the breakpoints of the symbol intervals
     * @param split_strategy_type Strategy for choosing the index to split on
     * @param num_bits_limit Maximum number of bits per segment
     */
    iSaxEnvelopeIndexParams(uint pos_per_env, uint segment_len, SaxNumBitsT first_layer_num_bits, size_t leaf_capacity,
                            iSaxBreakpointStrategyType breakpoint_strategy_type,
                            iSaxSplitStrategyType split_strategy_type, SaxNumBitsT num_bits_limit) {
        this->pos_per_env = pos_per_env;
        this->segment_len = segment_len;
        this->first_layer_num_bits = first_layer_num_bits;
        this->leaf_capacity = leaf_capacity;
        this->breakpoint_strategy_type = breakpoint_strategy_type;
        this->split_strategy_type = split_strategy_type;
        this->num_bits_limit = num_bits_limit;
    }
};

/** @brief Enumeration type for the cereal archives */
enum ArchiveType { BINARY, JSON };

/** @brief Map from strings to ArchiveType */
const umap<str, ArchiveType> STR_TO_ARCHIVE_TYPE = {{"binary", BINARY}, {"json", JSON}};

/** @brief Vector of accepted strings for STR_TO_ARCHIVE_TYPE */
const vec<str> ARCHIVE_TYPE_STRS = get_map_keys(STR_TO_ARCHIVE_TYPE);

/** @brief Options for creating an index */
struct IndexOptions {
    /** @brief Path to the dataset */
    str dataset_path;
    /** @brief Path to the index */
    str index_path;
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

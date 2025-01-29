#ifndef INDEX_OPTIONS_HPP
#define INDEX_OPTIONS_HPP

#include "Summarization/iSaxBreakpointStrategy.hpp"
#include "Search/iSax/iSaxSplitStrategy.hpp"
#include "util.hpp"

enum IndexType { ISAX_ENVELOPE };

const umap<str, IndexType> STR_TO_INDEX_TYPE = {{"isax_envelope", ISAX_ENVELOPE}};
const vec<str> INDEX_TYPE_STRS = get_keys(STR_TO_INDEX_TYPE);

const vec<IndexType> ENVELOPE_TYPES = {ISAX_ENVELOPE};

struct IIndexParams {
    virtual ~IIndexParams() = default;
    virtual IndexType get_type() const = 0;
};

struct EnvelopeIndexParams : IIndexParams {
    unsigned pos_per_env;
};

struct iSaxIndexParams {
    unsigned segment_len;
    SaxNumBitsT first_layer_num_bits;
    size_t leaf_capacity;
    iSaxBreakpointStrategyType breakpoint_strategy_type;
    iSaxSplitStrategyType split_strategy_type;
    SaxNumBitsT num_bits_limit;
};

struct iSaxEnvelopeIndexParams : EnvelopeIndexParams, iSaxIndexParams {
    IndexType get_type() const override { return ISAX_ENVELOPE; }

    iSaxEnvelopeIndexParams(unsigned pos_per_env, unsigned segment_len, SaxNumBitsT first_layer_num_bits,
                            size_t leaf_capacity, iSaxBreakpointStrategyType breakpoint_strategy_type,
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

enum ArchiveType { BINARY, JSON };
const umap<str, ArchiveType> STR_TO_ARCHIVE_TYPE = {{"binary", BINARY}, {"json", JSON}};
const vec<str> ARCHIVE_TYPE_STRS = get_keys(STR_TO_ARCHIVE_TYPE);

struct IndexOptions {
    std::string dataset_path, index_path;
    ArchiveType index_format;
    unsigned l_min, l_max, series_len, num_channels;
    bool normalized;
    std::unique_ptr<IIndexParams> index_params;
};

#endif  // INDEX_OPTIONS_HPP

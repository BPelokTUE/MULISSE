#ifndef INDEX_OPTIONS_HPP
#define INDEX_OPTIONS_HPP

#include "Summarization/iSaxBreakpointStrategy.hpp"
#include "Search/iSax/iSaxSplitStrategy.hpp"

enum IndexType { ISAX_ENVELOPE };

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
};

struct IndexOptions {
    std::string dataset_path, index_path;
    unsigned l_min, l_max, series_len, num_channels;
    bool normalized;
    std::unique_ptr<IIndexParams> index_params;
};

#endif  // INDEX_OPTIONS_HPP

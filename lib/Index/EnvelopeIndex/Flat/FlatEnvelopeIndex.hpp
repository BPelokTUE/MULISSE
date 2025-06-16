#ifndef INDEX_ENVELOPEINDEX_FLATENVELOPEINDEX_HPP
#define INDEX_ENVELOPEINDEX_FLATENVELOPEINDEX_HPP

#include "Index/EnvelopeIndex/EnvelopeIndex.hpp"

class IChannelSegmentationStrategy;

/**
 * @brief Flat envelope index
 * @tparam EnvT The type of envelope to store in the index, can be Envelope or SaxEnvelope
 * */
template <typename EnvT>
class FlatEnvelopeIndex : public EnvelopeIndex<EnvT>, public std::enable_shared_from_this<FlatEnvelopeIndex<EnvT>> {
   public:
    /**
     * @brief Construct a new FlatEnvelopeIndex instance
     * @param ch_segmentation_strategy The channel segmentation strategy to use
     * @param pos_per_env Number of positions per envelope
     * @param sax_num_bits Number of bits used for SAX discretization. Defaults to 0, indicating no discretization.
     * If greater than 0, the breakpoints are assumed to have the same cardinality.
     */
    FlatEnvelopeIndex(sptr<IChannelSegmentationStrategy> ch_segmentation_strategy, uint pos_per_env);

    FlatEnvelopeIndex() = default;

    void insert_entries(vec<IndexEntry<Envelope>> &entries, EntryInserterType inserter_type) override;

    void insert(IndexEntry<Envelope> &entry) override;

    uptr<IFinalizedIndex<EnvelopeTag>> finalize() override;
};

#endif  // INDEX_ENVELOPEINDEX_FLATENVELOPEINDEX_HPP

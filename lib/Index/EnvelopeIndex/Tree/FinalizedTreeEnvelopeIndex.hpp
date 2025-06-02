#ifndef INDEX_ENVELOPEINDEX_TREE_FINALIZEDTREEENVELOPEINDEX_HPP
#define INDEX_ENVELOPEINDEX_TREE_FINALIZEDTREEENVELOPEINDEX_HPP

#include "Index/EnvelopeIndex/FinalizedEnvelopeIndex.hpp"
#include "Index/EnvelopeIndex/Tree/EnvelopeNode.hpp"
#include "Index/Segmentation/ChannelSegmentationStrategy/ChannelSegmentationStrategy.hpp"
#include "Serialization/Macros.hpp"

class FinalizedTreeEnvelopeIndex : public FinalizedEnvelopeIndex {
   public:
    FinalizedTreeEnvelopeIndex() = default;

    /**
     * @brief Construct a new FinalizedTreeEnvelopeIndex instance
     * @param ch_segmentation_strategy The channel segmentation strategy to use
     * @param pos_per_env The number of positions per envelope
     * @param nodes The envelope nodes in the first layer of the tree
     */
    FinalizedTreeEnvelopeIndex(sptr<IChannelSegmentationStrategy> ch_segmentation_strategy, const uint pos_per_env,
                               vec<uptr<EnvelopeNode>> &&nodes);

    /**
     * @brief Get the first layer nodes of the tree
     * @return The first layer nodes of the tree
     */
    const vec<const EnvelopeNode *> get_first_layer_nodes() const;

   private:
    vec<uptr<EnvelopeNode>> m_first_layer_nodes;

    MAKE_SERIALIZABLE((m_ch_segmentation_strategy, m_pos_per_env, m_first_layer_nodes));
};

#endif  // INDEX_ENVELOPEINDEX_TREE_FINALIZEDTREEENVELOPEINDEX_HPP

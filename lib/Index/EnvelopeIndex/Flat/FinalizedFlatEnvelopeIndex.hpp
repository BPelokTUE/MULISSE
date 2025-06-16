#ifndef INDEX_ENVELOPEINDEX_FLAT_FINALIZEDFLATENVELOPEINDEX_HPP
#define INDEX_ENVELOPEINDEX_FLAT_FINALIZEDFLATENVELOPEINDEX_HPP

#include "Index/Entry/Envelope.hpp"
#include "Index/Entry/IndexEntry.hpp"
#include "Index/EnvelopeIndex/FinalizedEnvelopeIndex.hpp"
#include "Index/Segmentation/ChannelSegmentationStrategy/ChannelSegmentationStrategy.hpp"
#include "Serialization/Macros.hpp"

/**
 * @brief Finalized FlatEnvelopeIndex
 * @tparam EnvT The type of envelope to store in the index, can be Envelope or SaxEnvelope
 */
template <typename EnvT>
class FinalizedFlatEnvelopeIndex : public FinalizedEnvelopeIndex {
   public:
    FinalizedFlatEnvelopeIndex();

    /**
     * @brief Construct a new FinalizedFlatEnvelopeIndex instance
     * @param ch_segmentation_strategy The channel segmentation strategy to use
     * @param pos_per_env The number of positions per envelope
     * @param entries The envelope entries in the index
     */
    FinalizedFlatEnvelopeIndex(sptr<IChannelSegmentationStrategy> ch_segmentation_strategy, const uint pos_per_env,
                               vec<IndexEntry<EnvT>> &&entries);

    /**
     * @brief Get the entry of the index at the given index
     * @param ind The index of the entry to get
     * @return The entry at the given index
     */
    auto get_entry(uint ind) const -> decltype(auto) {
        if constexpr (std::is_same_v<EnvT, Envelope>) {
            // Return a reference to the entry at the given index
            return m_entries.at(ind);
        } else {
            // Return a new IndexEntry<Envelope> generated from the SaxEnvelope at the given index
            vec<Envelope> envelopes;
            envelopes.reserve(m_entries.at(ind).m_mts_summary.size());
            for (const auto &sax_env : m_entries.at(ind).m_mts_summary)
                envelopes.push_back(sax_env.to_envelope(*m_breakpoints));
            return IndexEntry<Envelope>{.m_subs_info = m_entries.at(ind).m_subs_info, .m_mts_summary = envelopes};
        }
    }

    /**
     * @brief Get the number of entries in the index
     * @return The number of entries in the index
     */
    uint size() const;

   private:
    vec<IndexEntry<EnvT>> m_entries;
    const vec<Real> *m_breakpoints = nullptr;

    MAKE_SERIALIZABLE((m_ch_segmentation_strategy, m_pos_per_env, m_entries));
};

#endif  // INDEX_ENVELOPEINDEX_FLAT_FINALIZEDFLATENVELOPEINDEX_HPP

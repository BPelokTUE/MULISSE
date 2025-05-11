#ifndef INDEX_ENVELOPEINDEX_FLATENVELOPEINDEX_HPP
#define INDEX_ENVELOPEINDEX_FLATENVELOPEINDEX_HPP

#include <queue>

#include "Enums/EntryInserterType.hpp"
#include "Index/Entry/Envelope.hpp"
#include "Index/Entry/IndexEntry.hpp"
#include "Index/EntryInserter/EntryInserter.hpp"
#include "Index/EntryInserter/TopDownInserter.hpp"
#include "Index/EnvelopeIndex/EnvelopeIndex.hpp"
#include "Index/EnvelopeIndex/Flat/FinalizedFlatEnvelopeIndex.hpp"
#include "Serialization/Macros.hpp"
#include "Util/RunSettings/RunSettings.hpp"
#include "Util/Types/Containers.hpp"

/** @brief Flat envelope index */
class FlatEnvelopeIndex : public EnvelopeIndex, public std::enable_shared_from_this<FlatEnvelopeIndex> {
   public:
    /**
     * @brief Construct a new FlatEnvelopeIndex instance
     * @param segmentation_strategy The segmentation strategy to use
     * @param pos_per_env Number of positions per envelope
     * @param sax_num_bits Number of bits used for SAX discretization. Defaults to 0, indicating no discretization.
     * If greater than 0, the breakpoints are assumed to have the same cardinality.
     */
    FlatEnvelopeIndex(sptr<ISegmentationStrategy> segmentation_strategy, uint pos_per_env, SaxNumBitsT sax_num_bits = 0)
        : EnvelopeIndex(segmentation_strategy, pos_per_env), m_sax_num_bits(sax_num_bits) {}

    FlatEnvelopeIndex() = default;

    void insert_entries(vec<IndexEntry<Envelope>> &entries, EntryInserterType inserter_type) override {
        uptr<IEntryInserter<FlatEnvelopeIndex>> inserter;
        switch (inserter_type) {
            case PARALLEL:  // Temporary solution to support two-stage indexes
            case TOP_DOWN:
                inserter = std::make_unique<TopDownInserter<FlatEnvelopeIndex>>(this->shared_from_this());
                break;
            default:
                throw std::invalid_argument("Invalid inserter type");
        }
        inserter->insert_entries(entries);
    };

    void insert(IndexEntry<Envelope> &entry) override {
        if (m_sax_num_bits > 0) {
            auto &breakpoints = RunSettings::get_instance().get_breakpoints();
            SaxSegIndT num_seg_per_channel = static_cast<SaxSegIndT>(entry.m_mts_summary[0].m_lower.size());

            for (MtsNumChannelsT c = 0; c < entry.m_mts_summary.size(); ++c) {
                SaxWord sax_lower(entry.m_mts_summary[c].m_lower, m_sax_num_bits, breakpoints);
                SaxWord sax_upper(entry.m_mts_summary[c].m_upper, m_sax_num_bits, breakpoints);

                for (SaxSegIndT s = 0; s < num_seg_per_channel; ++s) {
                    entry.m_mts_summary[c].m_lower[s] = sax_lower[s] > 0 ? breakpoints[sax_lower[s] - 1] : -INF;
                    entry.m_mts_summary[c].m_upper[s] =
                        sax_upper[s] < breakpoints.size() ? breakpoints[sax_upper[s]] : INF;
                }
            }
        }
        EnvelopeIndex::insert(entry);
    }

    uptr<IFinalizedIndex<EnvelopeTag>> finalize() override {
        return std::make_unique<FinalizedFlatEnvelopeIndex>(m_segmentation_strategy, m_pos_per_env,
                                                            std::move(m_entries));
    }

   protected:
    SaxNumBitsT m_sax_num_bits;
};

#endif  // INDEX_ENVELOPEINDEX_FLATENVELOPEINDEX_HPP

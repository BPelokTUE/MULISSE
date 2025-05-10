#ifndef ENVELOPE_ENTRY_MERGER_HPP
#define ENVELOPE_ENTRY_MERGER_HPP

#include "Util/typedefs.hpp"
#include "Util/utilities.hpp"
#include "Util/RunSettings.hpp"
#include "Summarization/IndexEntry.hpp"
#include "Summarization/Envelope.hpp"
#include "Summarization/SaxHelpers.hpp"

/** @brief Enum for IEnvelopeEntryMerger implementations */
enum EnvelopeEntryMergerType { DUMMY, SAX_BASED, LOWER_SAX_BASED };

DEFINE_ENUM_CONSTS(EnvelopeEntryMergerType, ENVELOPE_ENTRY_MERGER_TYPE, false,
                   (umap<str, EnvelopeEntryMergerType>{
                       {"sax", SAX_BASED}, {"lower_sax", LOWER_SAX_BASED}, {"none", DUMMY}}));

constexpr std::array<EnvelopeEntryMergerType, 2> MERGERS_W_SAX{SAX_BASED, LOWER_SAX_BASED};

/** @brief Interface for merging IndexEntry<Envelope> objects */
class IEnvelopeEntryMerger {
   public:
    virtual ~IEnvelopeEntryMerger() = default;

    /**
     * @brief Merge entries
     * @param entries The list of entries to merge. R-value reference.
     * @return The merged entries
     */
    virtual vec<IndexEntry<Envelope>> merge_entries(vec<IndexEntry<Envelope>> &&entries) = 0;
};

/** @brief Dummy implementation of IEnvelopeEntryMerger that does not do any merging */
class DummyEnvelopeEntryMerger : public IEnvelopeEntryMerger {
   public:
    vec<IndexEntry<Envelope>> merge_entries(vec<IndexEntry<Envelope>> &&entries) override;
};

/** @brief Class that merges overlapping IndexEntry<Envelope> objects if their SAX representation is the same */
class SaxBasedEnvelopeEntryMerger : public IEnvelopeEntryMerger {
   public:
    /**
     * @brief Constructor
     * @param sax_num_bits The number of bits to use for the SAX representation
     */
    SaxBasedEnvelopeEntryMerger(SaxNumBitsT sax_num_bits);

    vec<IndexEntry<Envelope>> merge_entries(vec<IndexEntry<Envelope>> &&entries) override;

   protected:
    /**
     * @brief Merge and add entries with the same symbols to the list of merged entries
     * @param symbol_entries The list of entries to merge
     * @param merged_entries The list of merged entries
     */
    void merge_and_add_entries(vec<IndexEntry<Envelope>> &&symbols_entries, vec<IndexEntry<Envelope>> &merged_entries);

   private:
    SaxNumBitsT m_sax_num_bits, m_alphabet_num_bits;
    const vec<Real> *m_breakpoints;

   protected:
    iSaxWordFactory m_isax_word_factory;
};

/** @brief Class that merges overlapping IndexEntry<Envelope> objects if the SAX representation of their lower bounds is
 * the same */
class LowerSaxBasedEnvelopeEntryMerger : public SaxBasedEnvelopeEntryMerger {
   public:
    /**
     * @brief Constructor
     * @param sax_num_bits The number of bits to use for the SAX representation
     */
    LowerSaxBasedEnvelopeEntryMerger(SaxNumBitsT sax_num_bits);

    vec<IndexEntry<Envelope>> merge_entries(vec<IndexEntry<Envelope>> &&entries) override;
};

#endif  // ENVELOPE_ENTRY_MERGER_HPP

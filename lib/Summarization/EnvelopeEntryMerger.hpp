#ifndef ENVELOPE_ENTRY_MERGER_HPP
#define ENVELOPE_ENTRY_MERGER_HPP

#include "Util/typedefs.hpp"
#include "Util/RunSettings.hpp"
#include "Summarization/IndexEntry.hpp"
#include "Summarization/Envelope.hpp"
#include "Summarization/SaxHelpers.hpp"

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

/** @brief Class that merges overlapping IndexEntry objects if their SAX representation is the same */
class SaxBasedEnvelopeEntryMerger : public IEnvelopeEntryMerger {
   public:
    SaxBasedEnvelopeEntryMerger() = default;

    /**
     * @brief Constructor
     * @param sax_num_bits The number of bits to use for the SAX representation
     */
    SaxBasedEnvelopeEntryMerger(SaxNumBitsT sax_num_bits);

    vec<IndexEntry<Envelope>> merge_entries(vec<IndexEntry<Envelope>> &&entries) override;

   private:
    SaxNumBitsT m_sax_num_bits, m_alphabet_num_bits;
    const vec<Real> *m_breakpoints;
    iSaxWordFactory m_isax_word_factory;
};

#endif  // ENVELOPE_ENTRY_MERGER_HPP

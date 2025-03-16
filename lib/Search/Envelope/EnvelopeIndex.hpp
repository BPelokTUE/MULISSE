#ifndef ENVELOPE_INDEX_HPP
#define ENVELOPE_INDEX_HPP

#include "Search/Index.hpp"
#include "Search/TopDownInserter.hpp"
#include "Search/Options/IndexOptions.hpp"
#include "Util/typedefs.hpp"

class FlatEnvelopeIndex : public IIndex<Envelope>,
                          public IFinalizedIndex<EnvelopeTag>,
                          public std::enable_shared_from_this<FlatEnvelopeIndex> {
   public:
    FlatEnvelopeIndex(uint segment_len, uint pos_per_env);

    FlatEnvelopeIndex() = default;

    void insert(const IndexEntry<Envelope> &entry) override;

    void insert_entries(const vec<IndexEntry<Envelope>> &entries, EntryInserterType inserter_type) override;

    uptr<IFinalizedIndex<EnvelopeTag>> finalize() override;

    vec<SearchResult> search(const vec<vec<float>> &query, const SearchOptions &opts,
                             std::ifstream &dataset_ifs) const override;

    const vec<IndexEntry<Envelope>> &get_entries() const;

   private:
    vec<IndexEntry<Envelope>> m_entries;
    uint m_segment_len, m_pos_per_env;

    MAKE_SERIALIZABLE((m_segment_len, m_pos_per_env, m_entries));
};

#endif  // ENVELOPE_INDEX_HPP

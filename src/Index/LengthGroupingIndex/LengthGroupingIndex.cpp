#include "Index/LengthGroupingIndex/LengthGroupingIndex.hpp"

#include "Index/Entry/IndexEntry.hpp"
#include "Index/LengthGroupingIndex/FinalizedLengthGroupingIndex.hpp"
#include "Index/Traits/EntryDataSpec.hpp"
#include "Util/HelperFuncs/Parallelism.hpp"
#include "Util/Logging/IndexLogger.hpp"

template <typename T>
LengthGroupingIndex<T>::LengthGroupingIndex(vec<sptr<IIndex<T>>> indexes, LengthProperties length_props)
    : m_indexes(indexes), m_length_props(length_props) {
    assert(U(m_indexes.size()) == m_length_props.m_num_l_groups);
}

template <typename T>
void LengthGroupingIndex<T>::insert_entry_groups(vec<vec<IndexEntry<T>>> &entry_groups,
                                                 EntryInserterType inserter_type) {
        OMP_PRAGMA(omp parallel for)
        for (uint l_ind = 0; l_ind < U(entry_groups.size()); ++l_ind) {
            auto &entry_group = entry_groups[l_ind];
            m_indexes[l_ind]->insert_entries(entry_group, inserter_type);
        }
}

template <typename T>
void LengthGroupingIndex<T>::insert_entries(vec<IndexEntry<T>> &entries, EntryInserterType inserter_type) {
    for (auto &entry : entries) {
        m_indexes[get_entry_length_group(entry)]->insert(entry);
    }
}

template <typename T>
void LengthGroupingIndex<T>::insert(IndexEntry<T> &entry) {
    m_indexes[get_entry_length_group(entry)]->insert(entry);
}

template <typename T>
uptr<IFinalizedIndex<typename IndexTraits<T>::FinalizedTag>> LengthGroupingIndex<T>::finalize() {
    using FTag = typename IndexTraits<T>::FinalizedTag;

    vec<uptr<IFinalizedIndex<FTag>>> finalized_indexes(m_indexes.size());
    for (uint l_ind = 0; l_ind < m_indexes.size(); ++l_ind) {
        finalized_indexes[l_ind] = m_indexes[l_ind]->finalize();
    }
    return uptr<IFinalizedIndex<FTag>>(
        new FinalizedLengthGroupingIndex<FTag>(std::move(finalized_indexes), m_length_props));
}

template <typename T>
void LengthGroupingIndex<T>::adapt_to_dataset_groups(const vec<vec<IndexEntry<T>>> &dataset_entry_groups) {
    for (uint l_ind = 0; l_ind < dataset_entry_groups.size(); ++l_ind) {
        m_indexes[l_ind]->adapt_to_dataset(dataset_entry_groups[l_ind]);
    }
}

template <typename T>
uint LengthGroupingIndex<T>::get_entry_length_group(const IndexEntry<T> &entry) const {
    return m_length_props.get_length_group(entry.m_subs_info.m_length);
}

// Template specializations
DECLARE_ENTRY_DATA_SPECS(LengthGroupingIndex);

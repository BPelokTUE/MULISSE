#include "Index/ChainIndex/FinalizedChainIndex.hpp"

#include "Index/Traits/EntryTags.hpp"

template <typename FTag>
FinalizedChainIndex<FTag>::FinalizedChainIndex(vec<uptr<IFinalizedIndex<FTag>>> approx_indexes,
                                               uptr<IFinalizedIndex<FTag>> exact_index)
    : m_approx_indexes(std::move(approx_indexes)), m_exact_index(std::move(exact_index)) {}

template <typename FTag>
void FinalizedChainIndex<FTag>::save(const str &out_file, ArchiveType ar_type) {
    for (uint approx_ind = 0; approx_ind < m_approx_indexes.size(); ++approx_ind)
        m_approx_indexes[approx_ind]->save(get_index_file_path(out_file, false, ar_type, approx_ind), ar_type);
    m_exact_index->save(get_index_file_path(out_file, true, ar_type), ar_type);
}

template <typename FTag>
void FinalizedChainIndex<FTag>::load(const str &in_file, ArchiveType ar_type) {
    for (uint approx_ind = 0; approx_ind < m_approx_indexes.size(); ++approx_ind)
        m_approx_indexes[approx_ind]->load(get_index_file_path(in_file, false, ar_type, approx_ind), ar_type);
    m_exact_index->load(get_index_file_path(in_file, true, ar_type), ar_type);
}

template <typename FTag>
size_t FinalizedChainIndex<FTag>::get_size_on_disk(const str &index_file, const ArchiveType ar_type) const {
    size_t size = 0;
    for (uint approx_ind = 0; approx_ind < m_approx_indexes.size(); ++approx_ind)
        size +=
            m_approx_indexes[approx_ind]->get_size_on_disk(get_index_file_path(index_file, false, ar_type, approx_ind));
    size += m_exact_index->get_size_on_disk(get_index_file_path(index_file, true, ar_type));
    return size;
}

template <typename FTag>
IFinalizedIndex<FTag> *FinalizedChainIndex<FTag>::release_approx_index(uint index) {
    assert(index < m_approx_indexes.size());
    return m_approx_indexes[index].release();
}

template <typename FTag>
IFinalizedIndex<FTag> *FinalizedChainIndex<FTag>::release_exact_index() {
    return m_exact_index.release();
}

DECLARE_ENTRY_TAG_SPECS(FinalizedChainIndex)

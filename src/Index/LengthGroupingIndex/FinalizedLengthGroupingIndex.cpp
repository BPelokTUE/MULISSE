#include "Index/LengthGroupingIndex/FinalizedLengthGroupingIndex.hpp"

#include "Util/HelperFuncs/Conversion.hpp"

using std::to_string;

template <typename FTag>
FinalizedLengthGroupingIndex<FTag>::FinalizedLengthGroupingIndex(vec<uptr<IFinalizedIndex<FTag>>> indexes,
                                                                 LengthProperties length_props)
    : m_indexes(std::move(indexes)), m_length_props(length_props) {
    assert(U(m_indexes.size()) == m_length_props.m_num_l_groups);
}

template <typename FTag>
void FinalizedLengthGroupingIndex<FTag>::save(const str &out_file, ArchiveType ar_type) {
    str base = get_file_base_and_extension(out_file).first;

    if (!fs::exists(base)) fs::create_directories(base);

    for (uint l_ind = 0; l_ind < m_indexes.size(); ++l_ind)
        m_indexes[l_ind]->save(fs::path(base) / get_index_file_name(l_ind), ar_type);

    // Temporary solution until metafiles are introduced
    m_length_props.save(fs::path(base) / LengthProperties::DEFAULT_FILE_NAME);
}

template <typename FTag>
void FinalizedLengthGroupingIndex<FTag>::load(const str &in_file, ArchiveType ar_type) {
    str base = get_file_base_and_extension(in_file).first;

    for (uint l_ind = 0; l_ind < m_indexes.size(); ++l_ind)
        m_indexes[l_ind]->load(fs::path(base) / get_index_file_name(l_ind), ar_type);
}

template <typename FTag>
size_t FinalizedLengthGroupingIndex<FTag>::get_size_on_disk(const str &index_file, const ArchiveType ar_type) const {
    str base = get_file_base_and_extension(index_file).first;

    size_t size = 0;
    for (uint l_ind = 0; l_ind < m_indexes.size(); ++l_ind)
        size += m_indexes[l_ind]->get_size_on_disk(fs::path(base) / get_index_file_name(l_ind), ar_type);
    return size;
}

template <typename FTag>
IFinalizedIndex<FTag> *FinalizedLengthGroupingIndex<FTag>::release_index(uint length_group) {
    assert(length_group < m_indexes.size());
    return m_indexes[length_group].release();
}

template <typename FTag>
str FinalizedLengthGroupingIndex<FTag>::get_index_file_name(uint length_group) const {
    uint lg_l_min = m_length_props.get_lg_l_min(length_group), lg_l_max = m_length_props.get_lg_l_max(length_group);
    return "LG_" + to_string(length_group) + "_" + to_string(lg_l_min) + "-" + to_string(lg_l_max);
}

// Template specializations
DECLARE_ENTRY_TAG_SPECS(FinalizedLengthGroupingIndex);

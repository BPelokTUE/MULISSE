#ifndef MODULES_INDEXING_GETISAXINDEX_HPP
#define MODULES_INDEXING_GETISAXINDEX_HPP

#include "Index/iSaxIndex/iSaxIndex.hpp"
#include "Modules/Indexing/IndexFactory/IndexFactoryParams.hpp"
#include "Modules/Indexing/StrategyFactory/GetSplitStrategy.hpp"

template <typename T>
    requires DerivedFromEntryData<T>
sptr<IIndex<T>> get_isax_index(const GeneralIndexProperties &opts, const iSaxIndexParams *params,
                               sptr<IChannelSegmentationStrategy> ch_segmentation_strategy,
                               uptr<IiSaxSplitStrategy<T>> split_strategy) {
    return std::make_shared<iSaxIndex<T>>(params->m_isax_trie_params.m_first_layer_num_bits,
                                          params->m_isax_trie_params.m_leaf_capacity, ch_segmentation_strategy,
                                          std::move(split_strategy), params->m_isax_trie_params.m_merge_in_leaves);
}

template <typename T>
    requires DerivedFromEntryData<T>
sptr<IIndex<T>> get_isax_index(IndexFactoryParams &factory_params) {
    const GeneralIndexProperties &opts = factory_params.m_opts;
    auto *index_params = dynamic_cast<iSaxIndexParams *>(opts.m_index_params.get());
    auto split_strategy = get_split_strategy<T>(index_params, opts.m_num_channels);

    return get_isax_index<T>(opts, index_params, factory_params.m_ch_segmentation_strategy, std::move(split_strategy));
}

#endif  // MODULES_INDEXING_GETISAXINDEX_HPP

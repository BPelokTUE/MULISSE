#ifndef MODULES_INDEXING_GETSPLITSTRATEGY_HPP
#define MODULES_INDEXING_GETSPLITSTRATEGY_HPP

#include "Index/IndexParams.hpp"
#include "Index/iSaxIndex/SplitStrategy/ClosestToMeanSplitStrategy.hpp"
#include "Index/iSaxIndex/SplitStrategy/EntropyMaximizingSplitStrategy.hpp"
#include "Index/iSaxIndex/SplitStrategy/RoundRobinSplitStrategy.hpp"
#include "Index/iSaxIndex/SplitStrategy/UlisseClosestToMeanSplitStrategy.hpp"

template <typename T>
uptr<IiSaxSplitStrategy<T>> get_split_strategy(const iSaxIndexParams *index_params, MtsNumChannelsT num_channels) {
    switch (index_params->m_isax_trie_params.m_split_strategy_type) {
        case DOUBLE_ROUND_ROBIN:
            return std::make_unique<RoundRobinSplitStrategy<T>>(index_params->m_segmentation_params.m_num_segments,
                                                                num_channels);
        case ENTROPY_MAXIMIZING:
            return std::make_unique<EntropyMaximizingSplitStrategy<T>>(
                index_params->m_isax_trie_params.m_min_num_bits_on_tie);
        case ULISSE_CLOSEST_TO_MEAN:
            return std::make_unique<UlisseClosestToMeanStrategy<T>>();
        case CLOSEST_TO_MEAN:
            return std::make_unique<ClosestToMeanSplitStrategy<T>>();
    }
    return nullptr;
}

#endif  // MODULES_INDEXING_GETSPLITSTRATEGY_HPP

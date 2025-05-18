#include "Index/iSaxIndex/SplitStrategy/RoundRobinSplitStrategy.hpp"

#include "Index/Traits/EntryDataSpec.hpp"
#include "Util/Types/SaxSplitIndex.hpp"

template <typename T>
RoundRobinSplitStrategy<T>::RoundRobinSplitStrategy(SaxSegIndT num_seg_per_channel, MtsNumChannelsT num_channels)
    : m_num_seg_per_channel(num_seg_per_channel), m_num_channels(num_channels) {}

template <typename T>
SaxSplitIndex RoundRobinSplitStrategy<T>::get_split_ind(const iSaxSplittableLeaf<T> *leaf,
                                                        const vec<iSaxWord> &isax_mins) {
    SaxSplitIndex inds = {m_current_split, m_current_channel};
    m_current_split = static_cast<SaxSegIndT>((m_current_split + 1) % m_num_seg_per_channel);
    m_current_channel = static_cast<SaxSegIndT>((m_current_channel + 1) % m_num_channels);
    return inds;
}

DECLARE_ENTRY_DATA_SPECS(RoundRobinSplitStrategy)

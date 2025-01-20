#include "Search/iSax/iSaxSplitStrategy.hpp"

DoubleRoundRobinStrategy::DoubleRoundRobinStrategy(SaxSegIndT num_seg_per_channel, MtsNumChannelsT num_channels)
    : m_num_seg_per_channel(num_seg_per_channel), m_num_channels(num_channels) {}

SaxSplitIndT DoubleRoundRobinStrategy::get_split_ind() {
    SaxSplitIndT inds = {m_current_split, m_current_channel};

    m_current_split = (m_current_split + 1) % m_num_seg_per_channel;
    m_current_channel = (m_current_channel + 1) % m_num_channels;

    return inds;
}

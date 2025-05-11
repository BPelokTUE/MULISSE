#ifndef UTIL_SAXSPLITINDEX_HPP
#define UTIL_SAXSPLITINDEX_HPP

#include <cereal/access.hpp>

#include "Util/Types/Numbers.hpp"

struct SaxSplitIndex {
    SaxSegIndT m_seg_ind;
    MtsNumChannelsT m_channel;

    bool operator==(const SaxSplitIndex &other) const {
        return m_seg_ind == other.m_seg_ind && m_channel == other.m_channel;
    }

    // Required for Cereal (de)serialization
    friend class cereal::access;

    template <class Archive>
    void serialize(Archive &ar) {
        ar(m_seg_ind, m_channel);
    }
};

#endif  // UTIL_SAXSPLITINDEX_HPP

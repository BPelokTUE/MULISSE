#ifndef INDEX_TRAITS_ENTRYTAGS_HPP
#define INDEX_TRAITS_ENTRYTAGS_HPP

#include "Util/Types/Containers.hpp"

struct PaaTag {};

struct EnvelopeTag {};

/**
 * @brief Concept to check if the entry type is valid for the index; TODO: this can be achieved without a concept
 * @tparam FTag The finalized traits tag
 */
template <typename FTag>
concept ValidEntryTraitsTag = std::is_same_v<FTag, PaaTag> || std::is_same_v<FTag, EnvelopeTag>;

#endif  // INDEX_TRAITS_ENTRYTAGS_HPP

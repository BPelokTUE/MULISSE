#ifndef INDEX_TRAITS_ENTRYTAGS_HPP
#define INDEX_TRAITS_ENTRYTAGS_HPP

#include "Util/Types/Containers.hpp"

struct PaaTag {};

struct EnvelopeTag {};

struct SaxEnvelopeTag {};

/**
 * @brief Declare template specializations for the Entry tags
 * @param CLASS The class to declare the specializations for
 */
#define DECLARE_ENTRY_TAG_SPECS(CLASS) \
    template class CLASS<PaaTag>;      \
    template class CLASS<EnvelopeTag>;

/**
 * @brief Concept to check if the entry type is valid for the index; TODO: this can be achieved without a concept
 * @tparam FTag The finalized traits tag
 */
template <typename FTag>
concept ValidEntryTraitsTag =
    std::is_same_v<FTag, PaaTag> || std::is_same_v<FTag, EnvelopeTag> || std::is_same_v<FTag, SaxEnvelopeTag>;

#endif  // INDEX_TRAITS_ENTRYTAGS_HPP

#ifndef INDEX_TRAITS_ENTRYDATASPEC_HPP
#define INDEX_TRAITS_ENTRYDATASPEC_HPP

#include "Index/Entry/Envelope.hpp"
#include "Index/Entry/Paa.hpp"

/**
 * @brief Declare template specializations for implementations IEntryData
 * @param CLASS The class to declare the specializations for
 */
#define DECLARE_ENTRY_DATA_SPECS(CLASS) \
    template class CLASS<Paa>;          \
    template class CLASS<Envelope>;

#endif  // INDEX_TRAITS_ENTRYDATASPEC_HPP

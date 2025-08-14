#ifndef ENUMS_ENTRYMERGERTYPE_HPP
#define ENUMS_ENTRYMERGERTYPE_HPP

#include "Util/HelperFuncs/Enums.hpp"
#include "Util/Types/String.hpp"
#include "Util/Types/UMap.hpp"

/** @brief Enum for IEntryMerger implementations */
enum EntryMergerType { DUMMY, SAX_BASED, LOWER_SAX_BASED, SAX_PAA_GENERATOR };

DEFINE_ENUM_CONSTS(EntryMergerType, ENTRY_MERGER_TYPE, false,
                   (umap<str, EntryMergerType>{{"sax", SAX_BASED},
                                               {"lower_sax", LOWER_SAX_BASED},
                                               {"none", DUMMY},
                                               {"paa_generator", SAX_PAA_GENERATOR},
                                               {"in_paa_generator", SAX_PAA_GENERATOR}}));

constexpr std::array MERGERS_W_SAX{SAX_BASED, LOWER_SAX_BASED, SAX_PAA_GENERATOR};

#endif  // ENUMS_ENTRYMERGERTYPE_HPP

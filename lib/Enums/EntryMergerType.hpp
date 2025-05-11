#ifndef ENUMS_ENTRYMERGERTYPE_HPP
#define ENUMS_ENTRYMERGERTYPE_HPP

#include "Util/HelperFuncs/Enums.hpp"
#include "Util/Types/Containers.hpp"

/** @brief Enum for IEntryMerger implementations */
enum EntryMergerType { DUMMY, SAX_BASED, LOWER_SAX_BASED };

DEFINE_ENUM_CONSTS(EntryMergerType, ENTRY_MERGER_TYPE, false,
                   (umap<str, EntryMergerType>{{"sax", SAX_BASED}, {"lower_sax", LOWER_SAX_BASED}, {"none", DUMMY}}));

constexpr std::array<EntryMergerType, 2> MERGERS_W_SAX{SAX_BASED, LOWER_SAX_BASED};

#endif  // ENUMS_ENTRYMERGERTYPE_HPP

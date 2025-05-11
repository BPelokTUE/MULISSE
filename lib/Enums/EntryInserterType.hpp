#ifndef INSERTER_TYPE_HPP
#define INSERTER_TYPE_HPP

#include "Util/HelperFuncs/Enums.hpp"
#include "Util/Types/Containers.hpp"

/** @brief Enum for IEntryInserter implementations */
enum EntryInserterType { TOP_DOWN, PARALLEL };

DEFINE_ENUM_CONSTS(EntryInserterType, ENTRY_INSERTER_TYPE, false,
                   (umap<str, EntryInserterType>{{"isax_parallel", PARALLEL}}));

#endif  // INSERTER_TYPE_HPP

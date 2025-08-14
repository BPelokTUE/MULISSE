#ifndef CLI_TRANSFORMERS_HPP
#define CLI_TRANSFORMERS_HPP

#include "CLI11/CLI11.hpp"
#include "Util/Types/String.hpp"
#include "Util/Types/UMap.hpp"

namespace transformers {
template <typename EnumType>
CLI::CheckedTransformer get_str_to_enum(const umap<str, EnumType> &enum_map);
};  // namespace transformers

#endif CLI_TRANSFORMERS_HPP

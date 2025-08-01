#include "CLI/Transformers.hpp"

namespace transformers {
template <typename EnumType>
CLI::CheckedTransformer get_str_to_enum(const umap<str, EnumType> &enum_map) {
    return CLI::CheckedTransformer(enum_map, CLI::ignore_case);
}
}  // namespace transformers

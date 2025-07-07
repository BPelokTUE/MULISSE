#ifndef MODULES_INDEXING_ESTIMATEFLATENVELOPEPARAMS_HPP
#define MODULES_INDEXING_ESTIMATEFLATENVELOPEPARAMS_HPP

#include <optional>

class IndexOptions;
class FlatEnvelopeParams;

std::optional<FlatEnvelopeParams> estimate_flat_envelope_params(IndexOptions &opts);

#endif  // MODULES_INDEXING_ESTIMATEFLATENVELOPEPARAMS_HPP

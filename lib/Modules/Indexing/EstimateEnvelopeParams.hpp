#ifndef MODULES_INDEXING_ESTIMATEENVELOPEPARAMS_HPP
#define MODULES_INDEXING_ESTIMATEENVELOPEPARAMS_HPP

#include <optional>

#include "Enums/DistanceType.hpp"

class IndexOptions;
class EnvelopeParams;

template <DistanceType D, bool EW>
std::optional<EnvelopeParams> estimate_envelope_params(IndexOptions &opts);

#endif  // MODULES_INDEXING_ESTIMATEENVELOPEPARAMS_HPP

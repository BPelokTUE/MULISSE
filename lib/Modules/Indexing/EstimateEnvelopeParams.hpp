#ifndef MODULES_INDEXING_ESTIMATEENVELOPEPARAMS_HPP
#define MODULES_INDEXING_ESTIMATEENVELOPEPARAMS_HPP

#include <optional>

#include "Enums/DistanceType.hpp"

class GeneralIndexProperties;
class EnvelopeParams;

template <DistanceType D, bool EW>
std::optional<EnvelopeParams> estimate_envelope_params(GeneralIndexProperties &opts);

#endif  // MODULES_INDEXING_ESTIMATEENVELOPEPARAMS_HPP

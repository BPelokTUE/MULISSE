#ifndef MODULES_INDEXING_ESTIMATEENVELOPEPARAMS_HPP
#define MODULES_INDEXING_ESTIMATEENVELOPEPARAMS_HPP

#include <optional>

class IndexOptions;
class EnvelopeParams;

std::optional<EnvelopeParams> estimate_envelope_params(IndexOptions &opts);

#endif  // MODULES_INDEXING_ESTIMATEENVELOPEPARAMS_HPP

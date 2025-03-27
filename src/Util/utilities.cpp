#include <cmath>
#include <fstream>

#include "Util/constants.hpp"
#include "Util/utilities.hpp"
#include "Util/typedefs.hpp"

size_t get_dataset_size(const str dataset_path) {
    std::ifstream data_stream(dataset_path, std::ios::binary | std::ios::ate);
    return data_stream.tellg();
}

std::pair<Real, Real> calculate_mu_and_sigma(Real sum, Real sum_sq, uint count) {
    Real mu = sum / count;
    Real sigma = std::sqrt(std::max(sum_sq / count - mu * mu, EPS_F));
    return {mu, sigma};
}

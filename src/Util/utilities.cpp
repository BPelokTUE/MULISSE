#include "Util/utilities.hpp"

#include <fstream>

size_t get_dataset_size(const str dataset_path) {
    std::ifstream data_stream(dataset_path, std::ios::binary | std::ios::ate);
    return data_stream.tellg();
}

std::pair<float, float> calculate_mu_and_sigma(float sum, float sum_sq, uint count) {
    float mu = sum / count;
    float sigma = std::sqrt(std::max(sum_sq / count - mu * mu, EPS_F));
    return {mu, sigma};
}

std::pair<double, double> calculate_mu_and_sigma(double sum, double sum_sq, uint count) {
    double mu = sum / count;
    double sigma = std::sqrt(std::max(sum_sq / count - mu * mu, EPS));
    return {mu, sigma};
}

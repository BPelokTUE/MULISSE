#include "Search/EnvelopeIndex.hpp"

void IEnvelopeIndex::construct(const str &dataset_path, IEnvelopeGenerator *generator, MtsNumChannelsT num_channels,
                               unsigned series_len) {
    m_dataset_path = dataset_path;

    unsigned N = get_dataset_size(dataset_path), series_size = num_channels * series_len * sizeof(float);
    unsigned num_series = N / series_size;

#pragma omp parallel
    {
        std::ifstream data_stream(dataset_path, std::ios::binary);
#pragma omp for
        for (size_t i = 0; i < num_series; ++i) {
            vec<vec<float>> mts(num_channels, vec<float>(series_len));
            data_stream.seekg(i * series_size);
            for (MtsNumChannelsT c = 0; c < num_channels; ++c) {
                data_stream.read(reinterpret_cast<char *>(mts[c].data()), series_len * sizeof(float));
            }

            auto entries = generator->get_entries(mts, i);
#pragma omp critical
            {
                for (auto entry : entries) insert(entry);
            }
        }
    }
}

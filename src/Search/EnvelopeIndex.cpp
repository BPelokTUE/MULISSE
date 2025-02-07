#include "Search/EnvelopeIndex.hpp"

uint IEnvelopeFinalizedIndex::get_series_len() const { return m_series_len; };

uint IEnvelopeFinalizedIndex::get_pos_per_env() const { return m_pos_per_env; };

MtsNumChannelsT IEnvelopeFinalizedIndex::get_num_channels() const { return m_num_channels; };

void IEnvelopeIndex::construct(const str &dataset_path, IEnvelopeGenerator *generator, MtsNumChannelsT num_channels,
                               uint series_len) {
    uint N = get_dataset_size(dataset_path), channel_size = series_len * sizeof(float),
         series_size = channel_size * num_channels;
    uint num_series = N / series_size;

    vec<EnvelopeEntry> dataset_entries;

#pragma omp parallel
    {
        std::ifstream data_stream(dataset_path, std::ios::binary);
#pragma omp for
        for (size_t i = 0; i < num_series; ++i) {
            vec<vec<float>> mts(num_channels, vec<float>(series_len));
            data_stream.seekg(i * series_size);
            for (MtsNumChannelsT c = 0; c < num_channels; ++c) {
                data_stream.read(reinterpret_cast<char *>(mts[c].data()), channel_size);
            }
            auto mts_entries = generator->get_entries(mts, i);
#pragma omp critical
            {
                dataset_entries.insert(dataset_entries.end(), mts_entries.begin(), mts_entries.end());
            }
        }
    }

    // TODO: adapt stuff based on entries, e.g. change breakpoint distribution mean

    for (auto &entry : dataset_entries) insert(std::move(entry));
}

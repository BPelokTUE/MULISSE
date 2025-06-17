#include "Index/Summarization.hpp"

#include "Index/Entry/Envelope.hpp"
#include "Index/Entry/IndexEntry.hpp"
#include "Index/Entry/Paa.hpp"
#include "Index/EntryGenerator/EntryGenerator.hpp"
#include "Index/EntryMerger/EntryMerger.hpp"
#include "Util/HelperFuncs/Conversion.hpp"
#include "Util/HelperFuncs/Parallelism.hpp"
#include "Util/RunSettings/RunSettings.hpp"

template <typename T>
vec<vec<IndexEntry<T>>> summarize_dataset(uint num_length_groups, uint num_series, const vec<uint> &mts_inds,
                                          uptr<IEntryGenerator<T>> generator, uptr<IEntryMerger<T>> merger) {
    vec<vec<IndexEntry<T>>> dataset_entry_groups(num_length_groups);
    auto &RS = RunSettings::get_instance();
    str dataset_path = RS.get_dataset_path();

    uint series_len = RS.get_dataset_props().m_series_len;
    MtsNumChannelsT num_channels = RS.get_dataset_props().m_num_channels;
    size_t channel_size = series_len * sizeof(Real), series_size = channel_size * num_channels;

    OMP_PRAGMA(omp parallel) {
        std::ifstream data_stream(dataset_path, std::ios::binary);
        OMP_PRAGMA(omp for)
        for (size_t i = 0; i < num_series; ++i) {
            uint mts_ind = mts_inds[i];
            vec<vec<Real>> mts(num_channels, vec<Real>(series_len));
            data_stream.seekg(static_cast<std::streamsize>(mts_ind * series_size));
            for (MtsNumChannelsT c = 0; c < num_channels; ++c) {
                data_stream.read(reinterpret_cast<char *>(mts[c].data()), static_cast<std::streamsize>(channel_size));
            }
            auto mts_entries = generator->get_entries(mts, U(mts_ind));
            for (uint l = 0; l < num_length_groups; ++l) {
                mts_entries[l] = merger->merge_entries(std::move(mts_entries[l]));
            }
            OMP_PRAGMA(omp critical) {
                for (uint l = 0; l < num_length_groups; ++l) {
                    dataset_entry_groups[l].insert(dataset_entry_groups[l].end(),
                                                   std::make_move_iterator(mts_entries[l].begin()),
                                                   std::make_move_iterator(mts_entries[l].end()));
                }
            }
        }
    }
    return dataset_entry_groups;
}

// Explicit instantiations of the template for required types
template vec<vec<IndexEntry<Paa>>> summarize_dataset<Paa>(uint, uint, const vec<uint> &, uptr<IEntryGenerator<Paa>>,
                                                          uptr<IEntryMerger<Paa>>);

template vec<vec<IndexEntry<Envelope>>> summarize_dataset<Envelope>(uint, uint, const vec<uint> &,
                                                                    uptr<IEntryGenerator<Envelope>>,
                                                                    uptr<IEntryMerger<Envelope>>);

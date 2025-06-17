#include "Index/Index.hpp"

#include "Index/Entry/IndexEntry.hpp"
#include "Index/EntryGenerator/EntryGenerator.hpp"
#include "Index/EntryMerger/EntryMerger.hpp"
#include "Index/Summarization.hpp"
#include "Index/Traits/EntryDataSpec.hpp"
#include "Util/HelperFuncs/Parallelism.hpp"
#include "Util/Logging/IndexLogger.hpp"
#include "Util/RunSettings/RunSettings.hpp"

template <typename T>
void IIndex<T>::construct(const str &dataset_path, uptr<IEntryGenerator<T>> generator, uptr<IEntryMerger<T>> merger,
                          EntryInserterType inserter_type, MtsNumChannelsT num_channels, uint series_len, bool adapt,
                          Real sample_frac) {
    auto &logger = IndexLogger::get_instance();
    auto &RS = RunSettings::get_instance();

    uint num_series = RS.get_dataset_props().m_num_series;

    vec<uint> mts_inds(num_series);
    std::iota(mts_inds.begin(), mts_inds.end(), 0);

    assert(sample_frac >= 0.0 && sample_frac <= 1.0);
    if (sample_frac < 1.0) {
        num_series = U(R(num_series) * sample_frac);
        std::shuffle(mts_inds.begin(), mts_inds.end(), std::mt19937{std::random_device{}()});
    }

    uint num_length_groups = RunSettings::get_instance().get_length_props().m_num_l_groups;

    logger.start_timer(ISC::SUMMARIZATION_TIME_S);
    auto dataset_entry_groups =
        summarize_dataset<T>(num_length_groups, num_series, mts_inds, std::move(generator), std::move(merger));
    logger.stop_timer(ISC::SUMMARIZATION_TIME_S);

    // TODO: adapt for length groups
    logger.increment_count_col(ISC::NUM_ENTRIES, dataset_entry_groups[0].size());

    if (adapt) adapt_to_dataset_groups(dataset_entry_groups);

    logger.start_timer(ISC::INSERTION_TIME_S);
    insert_entry_groups(dataset_entry_groups, inserter_type);
    logger.stop_timer(ISC::INSERTION_TIME_S);
}

DECLARE_ENTRY_DATA_SPECS(IIndex)

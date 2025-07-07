#include "Modules/Indexing/ConstructIndex.hpp"

#include "Index/Entry/Envelope.hpp"
#include "Index/Entry/Paa.hpp"
#include "Index/EntryGenerator/EntryGenerator.hpp"
#include "Index/EntryMerger/EntryMerger.hpp"
#include "Index/Estimator/IndexSizeEstimator.hpp"
#include "Index/FinalizedIndex.hpp"
#include "Index/Index.hpp"
#include "Index/LengthGroupingIndex/LengthGroupingIndex.hpp"
#include "Index/Segmentation/LengthGroupSegmentationStrategy/LengthGroupSegmentationStrategy.hpp"
#include "Util/Logging/IndexLogger.hpp"
#include "Util/RunSettings/RunSettings.hpp"

template <typename T>
sptr<IIndex<T>> get_index_without_data(std::function<sptr<IIndex<T>>(IndexFactoryParams &)> index_factory,
                                       const IndexOptions &opts,
                                       const ILengthGroupSegmentationStrategy *lg_segmentation_strategy) {
    // Temporary solution until metafile are introduced
    LengthProperties length_props{
        .m_use_length_groups = opts.m_use_length_groups,
        .m_l_min = opts.m_l_min,
        .m_l_max = opts.m_l_max,
    };
    length_props.set_lengths_per_group(opts.m_l_per_group);
    //

    sptr<IIndex<T>> index;
    if (opts.m_use_length_groups) {
        vec<sptr<IIndex<T>>> group_indexes(length_props.m_num_l_groups);
        for (uint lg_ind = 0; lg_ind < length_props.m_num_l_groups; lg_ind++) {
            IndexFactoryParams lg_factory_params{
                .m_ch_segmentation_strategy = lg_segmentation_strategy->get_ch_segmentation_strategy(lg_ind),
                .m_opts = opts,
            };
            group_indexes[lg_ind] = index_factory(lg_factory_params);
        }
        index = std::make_shared<LengthGroupingIndex<T>>(std::move(group_indexes), length_props);
    } else {
        IndexFactoryParams factory_params{
            .m_ch_segmentation_strategy = lg_segmentation_strategy->get_ch_segmentation_strategy(0),
            .m_opts = opts,
        };
        index = index_factory(factory_params);
    }
    return std::move(index);
}

template <typename T>
void construct_index(std::function<sptr<IIndex<T>>(IndexFactoryParams &)> index_factory,
                     uptr<IEntryGenerator<T>> generator, uptr<IEntryMerger<T>> merger, const IndexOptions &opts,
                     uptr<ILengthGroupSegmentationStrategy> lg_segmentation_strategy, Real sample_frac) {
    auto &RS = RunSettings::get_instance();
    auto &logger = IndexLogger::get_instance();

    auto index = get_index_without_data<T>(index_factory, opts, lg_segmentation_strategy.get());
    index->construct(RS.get_dataset_path(), std::move(generator), std::move(merger), opts.m_inserter_type,
                     opts.m_num_channels, opts.m_series_len, opts.m_adapt, sample_frac);
    auto finalized_index = index->finalize();
    finalized_index->save(RS.get_index_path(), opts.m_index_format);

    logger.increment_count_col(ISC::SIZE_ON_DISK_B, finalized_index->get_size_on_disk(RS.get_index_path()));
    if (arr_contains(METHODS_W_ESTIMABLE_SIZE, opts.m_index_method)) {
        IndexSizeEstimator size_estimator(opts.m_index_method, RS.get_length_props(), lg_segmentation_strategy.get());
        logger.increment_count_col(ISC::ESTIMATED_SIZE_ON_DISK_B, size_estimator.get_estimated_flat_envelope_size(
                                                                      RS.get_envelope_props().m_pos_per_env));
    }
}

// Template specializations
template void construct_index<Paa>(std::function<sptr<IIndex<Paa>>(IndexFactoryParams &)>, uptr<IEntryGenerator<Paa>>,
                                   uptr<IEntryMerger<Paa>>, const IndexOptions &,
                                   uptr<ILengthGroupSegmentationStrategy>, Real);

template void construct_index<Envelope>(std::function<sptr<IIndex<Envelope>>(IndexFactoryParams &)>,
                                        uptr<IEntryGenerator<Envelope>>, uptr<IEntryMerger<Envelope>>,
                                        const IndexOptions &, uptr<ILengthGroupSegmentationStrategy>, Real);

#ifndef MODULES_INDEXING_CONSTRUCTINDEX
#define MODULES_INDEXING_CONSTRUCTINDEX

#include "Index/Index.hpp"
#include "Index/LengthGroupingIndex/LengthGroupingIndex.hpp"
#include "Index/Segmentation/LengthGroupSegmentationStrategy/LengthGroupSegmentationStrategy.hpp"
#include "Modules/Indexing/IndexFactory/IndexFactoryParams.hpp"
#include "Util/Stats/IndexSizeEstimator.hpp"

template <typename T>
    requires DerivedFromEntryData<T>
void construct_index(std::function<sptr<IIndex<T>>(IndexFactoryParams &)> index_factory,
                     uptr<IEntryGenerator<T>> generator, uptr<IEntryMerger<T>> merger,
                     IndexFactoryParams &factory_params,
                     uptr<ILengthGroupSegmentationStrategy> lg_segmentation_strategy, Real sample_frac = 1.0) {
    auto &RS = RunSettings::get_instance();
    auto &logger = IndexLogger::get_instance();
    auto &opts = factory_params.m_opts;

    sptr<IIndex<T>> index;
    if (opts.m_use_length_groups) {
        uint num_len_groups = RS.get_length_props().m_num_l_groups;

        vec<sptr<IIndex<T>>> group_indexes(num_len_groups);
        for (uint lg_ind = 0; lg_ind < num_len_groups; lg_ind++) {
            IndexFactoryParams lg_factory_params{
                .m_discretize_flat_index = factory_params.m_discretize_flat_index,
                .m_ch_segmentation_strategy = lg_segmentation_strategy->get_ch_segmentation_strategy(lg_ind),
                .m_opts = opts,
            };
            group_indexes[lg_ind] = index_factory(lg_factory_params);
        }
        index = std::make_shared<LengthGroupingIndex<T>>(std::move(group_indexes), opts.m_l_min, opts.m_l_max);
    } else {
        factory_params.m_ch_segmentation_strategy = lg_segmentation_strategy->get_ch_segmentation_strategy(0);
        index = index_factory(factory_params);
    }

    index->construct(RS.get_dataset_path(), std::move(generator), std::move(merger), opts.m_inserter_type,
                     opts.m_num_channels, opts.m_series_len, opts.m_adapt, sample_frac);
    auto finalized_index = index->finalize();
    finalized_index->save(RS.get_index_path(), opts.m_index_format);

    logger.increment_count_col(ISC::SIZE_ON_DISK_B, finalized_index->get_size_on_disk(RS.get_index_path()));
    if (opts.m_index_method == ENVELOPE) {
        logger.increment_count_col(
            ISC::ESTIMATED_SIZE_ON_DISK_B,
            get_estimated_flat_envelope_size(lg_segmentation_strategy.get(), RS.get_envelope_props().m_pos_per_env));
    }
}

#endif  // MODULES_INDEXING_CONSTRUCTINDEX

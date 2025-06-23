#include "Modules/Indexing/IndexFactory/GetTreeEnvelopeIndex.hpp"

#include "Index/EnvelopeIndex/Grouping/EnvelopeGrouper.hpp"
#include "Index/EnvelopeIndex/Grouping/InvSaxSortingEnvelopeGrouper.hpp"
#include "Index/EnvelopeIndex/Grouping/RecursiveBucketingEnvelopeGrouper.hpp"
#include "Index/EnvelopeIndex/Grouping/VarianceLimitingEnvelopeGrouper.hpp"
#include "Index/EnvelopeIndex/Tree/TreeEnvelopeIndex.hpp"

sptr<IIndex<Envelope>> get_envelope_tree_index(IndexFactoryParams &factory_params) {
    const IndexOptions &opts = factory_params.m_opts;
    auto *index_params = dynamic_cast<TreeEnvelopeIndexParams *>(opts.m_index_params.get());

    uptr<IEnvelopeGrouper> grouper = nullptr;

    switch (opts.m_index_method) {
        case TREE_ENVELOPE:
            grouper =
                std::make_unique<RecursiveBucketingEnvelopeGrouper>(index_params->m_env_grouping_params.m_bucket_size);
            break;
        case BUCKETING_ENVELOPE:
            grouper = std::make_unique<BucketingEnvelopeGrouper>(index_params->m_env_grouping_params.m_bucket_size);
            break;
        case VL_ENVELOPE:
            grouper = std::make_unique<VarianceLimitingEnvelopeGrouper>(
                index_params->m_env_grouping_params.m_max_width_change);
            break;
        default:
            throw std::runtime_error("Invalid index method for TreeEnvelopeIndex");
    }

    if (index_params->m_env_grouping_params.m_use_inv_sax_sorting) {
        if (index_params->m_sax_params.m_num_bits == 0) {
            throw std::runtime_error("InvSAX sorting requires a positive number of bits");
        }
        grouper =
            std::make_unique<InvSaxSortingEnvelopeGrouper>(std::move(grouper), index_params->m_sax_params.m_num_bits);
    }
    return std::make_shared<TreeEnvelopeIndex>(factory_params.m_ch_segmentation_strategy, index_params->m_pos_per_env,
                                               std::move(grouper));
}

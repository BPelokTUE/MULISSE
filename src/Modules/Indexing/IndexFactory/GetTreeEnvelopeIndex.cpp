#include "Modules/Indexing/IndexFactory/GetTreeEnvelopeIndex.hpp"

#include "Index/EnvelopeIndex/Grouping/BucketingEnvelopeGrouper.hpp"
#include "Index/EnvelopeIndex/Grouping/EnvelopeGrouper.hpp"
#include "Index/EnvelopeIndex/Grouping/InvSaxSortingEnvelopeGrouper.hpp"
#include "Index/EnvelopeIndex/Tree/TreeEnvelopeIndex.hpp"

sptr<IIndex<Envelope>> get_envelope_tree_index(IndexFactoryParams &factory_params) {
    const IndexOptions &opts = factory_params.m_opts;
    auto *index_params = dynamic_cast<TreeEnvelopeIndexParams *>(opts.m_index_params.get());

    uptr<IEnvelopeGrouper> grouper = std::make_unique<InvSaxSortingEnvelopeGrouper>(
        std::make_unique<BucketingEnvelopeGrouper>(index_params->m_bucket_size), index_params->m_sax_params.m_num_bits);

    return std::make_shared<TreeEnvelopeIndex>(factory_params.m_ch_segmentation_strategy, index_params->m_pos_per_env,
                                               std::move(grouper));
}

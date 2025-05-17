#include "Modules/Indexing/IndexFactory/GetTreeEnvelopeIndex.hpp"

#include "Index/EnvelopeIndex/EnvelopeGrouper.hpp"
#include "Index/EnvelopeIndex/Tree/TreeEnvelopeIndex.hpp"

sptr<IIndex<Envelope>> get_envelope_tree_index(IndexFactoryParams &factory_params) {
    const IndexOptions &opts = factory_params.m_opts;
    auto *index_params = dynamic_cast<TreeEnvelopeIndexParams *>(opts.m_index_params.get());

    auto grouper =
        new InvSaxSortingBucketingEnvelopeGrouper(index_params->m_sax_params.m_num_bits, index_params->m_bucket_size);
    auto *index = new TreeEnvelopeIndex(factory_params.m_ch_segmentation_strategy, index_params->m_pos_per_env,
                                        uptr<IEnvelopeGrouper>(grouper));
    return sptr<IIndex<Envelope>>(index);
}

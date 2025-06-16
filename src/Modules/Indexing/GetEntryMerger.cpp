#include "Modules/Indexing/GetEntryMerger.hpp"

#include "Index/IndexOptions.hpp"

template <typename T>
uptr<IEntryMerger<T>> get_entry_merger_impl(const IndexOptions &opts) {
    auto *params = dynamic_cast<const PaaIndexParams *>(opts.m_index_params.get());
    switch (params->m_merger_params.m_entry_merger_type) {
        case DUMMY:
            return std::make_unique<DummyEntryMerger<T>>();
        case SAX_BASED:
            return std::make_unique<SaxBasedEntryMerger<T>>(params->m_merger_params.m_merger_sax_params->m_num_bits);
        case LOWER_SAX_BASED:
            if constexpr (std::is_same_v<T, Paa>) {
                throw std::runtime_error(
                    "LowerSaxBasedEntryMerger is only available for indexes with Envelope entries");
            } else {
                return std::make_unique<LowerSaxBasedEntryMerger>(
                    params->m_merger_params.m_merger_sax_params->m_num_bits);
            }
        case SAX_PAA_GENERATOR:
            if constexpr (std::is_same_v<T, Paa>) {
                return std::make_unique<DummyEntryMerger<Paa>>();
            } else {
                throw std::runtime_error("SAX-merging PAA generator is only available for indexes with Paa entries");
            }
    }
    return nullptr;
}

template <>
uptr<IEntryMerger<Paa>> get_entry_merger<Paa>(const IndexOptions &opts) {
    return get_entry_merger_impl<Paa>(opts);
}

template <>
uptr<IEntryMerger<Envelope>> get_entry_merger<Envelope>(const IndexOptions &opts) {
    return get_entry_merger_impl<Envelope>(opts);
}

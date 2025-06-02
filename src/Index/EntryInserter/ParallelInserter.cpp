#include "Index/EntryInserter/ParallelInserter.hpp"

#include "Index/Traits/EntryDataSpec.hpp"
#include "Util/HelperFuncs/Parallelism.hpp"

template <typename T>
iSaxParallelInserter<T>::iSaxParallelInserter(sptr<iSaxIndex<T>> index) : m_index(index) {}

template <typename T>
void iSaxParallelInserter<T>::insert_entries(vec<IndexEntry<T>> &entries) {
    assert(!entries.empty());

    umap_hash<vec<vec<SaxSymbolT>>, vec<uint>, SaxSymbolsHash> symbols_to_entry_inds;

    MtsNumChannelsT num_channels = static_cast<MtsNumChannelsT>(entries[0].m_mts_summary.size());
    SaxSegIndT num_segments = static_cast<SaxSegIndT>(entries[0].m_mts_summary[0].size());

        OMP_PRAGMA(omp parallel for)
        for (uint e_ind = 0; e_ind < U(entries.size()); ++e_ind) {
            vec<vec<SaxSymbolT>> symbols(num_channels, vec<SaxSymbolT>(num_segments));
            m_index->calculate_first_layer_symbols(entries[e_ind], symbols);

            OMP_PRAGMA(omp critical) {
                auto first_layer_node_it = m_index->m_first_layer.find(symbols);
                if (first_layer_node_it == m_index->m_first_layer.end()) {
                    m_index->insert_new_first_layer_node(symbols, entries[e_ind]);
                } else {
                    symbols_to_entry_inds[symbols].push_back(e_ind);
                }
            }
        }

        OMP_PRAGMA(omp parallel for)
        for (size_t bucket = 0; bucket < symbols_to_entry_inds.bucket_count(); ++bucket) {
            for (auto it = symbols_to_entry_inds.begin(bucket); it != symbols_to_entry_inds.end(bucket); ++it) {
                const auto &symbols = it->first;
                auto first_layer_node_it = m_index->m_first_layer.find(symbols);
                for (uint e_ind : it->second) {
                    auto isax_words = m_index->get_entry_isax(entries[e_ind]);
                    m_index->insert_into_first_layer_node(isax_words, first_layer_node_it, entries[e_ind]);
                }
            }
        }
}

DECLARE_ENTRY_DATA_SPECS(iSaxParallelInserter)

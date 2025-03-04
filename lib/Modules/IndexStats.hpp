#ifndef INDEX_STATS_HPP
#define INDEX_STATS_HPP

#include "Search/Options/SearchMethodType.hpp"
#include "Search/Options/IndexOptions.hpp"
#include "Search/Index.hpp"

template <typename IndexType, typename FTag>
concept ValidIndexType = std::is_base_of<IFinalizedIndex<FTag>, IndexType>::value;

template <typename IndexType, typename FTag>
    requires ValidIndexType<IndexType, FTag>
class IndexAnalyzer {
   public:
    IndexAnalyzer(uptr<IndexType> index) : m_index(std::move(index)) {};

    void analyze();

   private:
    uptr<IndexType> m_index;
};

int calculate_index_stats(SearchMethodType method_type, ArchiveType index_format);

#endif  // INDEX_STATS_HPP

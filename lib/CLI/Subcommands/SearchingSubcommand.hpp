#ifndef CLI_SUBCOMMANDS_SEARCHINGSUBCOMMAND_HPP
#define CLI_SUBCOMMANDS_SEARCHINGSUBCOMMAND_HPP

#include "CLI/Subcommands/Subcommand.hpp"
#include "CLI11/CLI11.hpp"
#include "Enums/DistanceType.hpp"
#include "Enums/SearchType.hpp"

class SearchingSubcommand : public ISubcommand {
   public:
    /**
     * @brief Constructor
     * @param app The CLI application to add the subcommand to
     */
    SearchingSubcommand(CLI::App &app);

    void execute() override;

   private:
    bool m_early_abandon = false, m_sort_query = false, m_examine_whole = false, m_no_use_pq = false,
         m_approximate = false;
    uint m_max_leaves_to_visit = 0, m_knn_k = 1, m_r_range_r = 10.0;
    str m_index_meta_path = "", m_dataset_meta_path = "", m_query_meta_path = "", m_ffts_meta_path = "",
        m_distance_measure_str = DISTANCE_TYPE_TO_STR.at(ED), m_search_type_str = SEARCH_TYPE_TO_STR.at(KNN);
};

#endif  // CLI_SUBCOMMANDS_SEARCHINGSUBCOMMAND_HPP

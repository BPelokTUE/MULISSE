#ifndef UTIL_ARTEFACTS_OPTIONS_CSVDATASETGENOPTIONS_HPP
#define UTIL_ARTEFACTS_OPTIONS_CSVDATASETGENOPTIONS_HPP

#include "Util/Types/LengthRange.hpp"
#include "Util/Types/Numbers.hpp"
#include "Util/Types/String.hpp"
#include "Util/Types/Vec.hpp"

struct CsvDatasetGenOptions {
    /** @brief Column separator in the csv files */
    char m_col_sep = ',';
    /** @brief The random seed to use */
    int m_seed = 0;
    /** @brief The minimum standard deviation allowed for any relevant-length subsequences */
    Real m_min_subs_sd;
    /** @brief The length range of subsequences to check standard deviation for */
    LengthRange m_l_range;
    /** @brief The source CSV files used for generating the dataset */
    vec<str> m_source_csvs;
};

#endif  // UTIL_ARTEFACTS_OPTIONS_CSVDATASETGENOPTIONS_HPP

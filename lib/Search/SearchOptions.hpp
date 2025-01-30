#ifndef SEARCH_OPTIONS_HPP
#define SEARCH_OPTIONS_HPP

/** @brief Types of  */
enum SearchType { KNN, R_RANGE };

/** @brief Interface for search parameters */
struct ISearchParams {
    virtual ~ISearchParams() = default;

    /**
     * @brief Get the type of the search parameters
     *
     * @return The type of the search parameters
     */
    virtual SearchType get_type() const = 0;
};

/** @brief Parameters for k Nearest Neighbor (kNN) search */
struct KnnParameters : ISearchParams {
    /** @brief Number of neighbors to retrieve */
    unsigned k;

    SearchType get_type() const override { return KNN; }
};

struct RRangeParameters : ISearchParams {
    /** @brief Range to find neighbors within */
    float r;

    SearchType get_type() const override { return R_RANGE; }
};

/** @brief Options for searching */
struct SearchOptions {
    /** @brief Whether to run exact or approximate search */
    bool exact = true;
    /** @brief Whether to Z-normalize or not */
    bool normalized = true;
    /** @brief Search parameters */
    uptr<ISearchParams> search_params;
};

#endif  // SEARCH_OPTIONS_HPP

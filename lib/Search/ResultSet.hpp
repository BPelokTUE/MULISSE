#ifndef RESULT_SET_HPP
#define RESULT_SET_HPP

#include "typedefs.hpp"

/** @brief Search result */
struct SearchResult {
    /** @brief Starting position of the first channel of the result in the dataset file */
    FilePositionT file_position;
    /** @brief Distance of the result to the query */
    DistanceT distance;
};

/** @brief Interface for result sets */
class IResultSet {
   public:
    virtual ~IResultSet() = default;

    /**
     * @brief Insert a search result into the result set
     *
     * @param result The search result to insert
     */
    virtual void insert(SearchResult result) = 0;

    /**
     * @brief Get the results in the result set
     *
     * @return The results in the result set
     */
    virtual vec<SearchResult> get_results() const = 0;

    /**
     * @brief Get the lower bound distance of the result set
     *
     * @return The lower bound distance of the result set; No result with a greater distance should be considered
     */
    virtual DistanceT get_distance_lb() const = 0;
};

/** @brief R-range result set */
class RRangeResultSet : public IResultSet {
   public:
    /**
     * @brief Construct a new RRangeResultSet object
     *
     * @param r The range to find neighbors within
     */
    RRangeResultSet(DistanceT r);

    void insert(SearchResult result) override {};

    vec<SearchResult> get_results() const override { return m_results; };

    DistanceT get_distance_lb() const override { return m_r; };

   private:
    vec<SearchResult> m_results;
    DistanceT m_r;
};

/** @brief K-Nearest-Neighbor (kNN) result set */
class KnnResultSet : public IResultSet {
   public:
    /**
     * @brief Construct a new KnnResultSet object
     *
     * @param k The number of neighbors to retrieve
     */
    KnnResultSet(unsigned k);

    void insert(SearchResult result) override {};

    vec<SearchResult> get_results() const override { return m_results; };

    DistanceT get_distance_lb() const override { return m_results.empty() ? 0 : m_results.back().distance; };

   private:
    vec<SearchResult> m_results;
    unsigned m_k;
};

#endif  // RESULT_SET_HPP

#ifndef RESULT_SET_HPP
#define RESULT_SET_HPP

#include "Util/typedefs.hpp"
#include "Util/utilities.hpp"

/** @brief Types of similarity search */
enum SearchType { KNN, R_RANGE };

DEFINE_ENUM_CONSTS_NO_EXTRA(SearchType, SEARCH_TYPE, false);

/** @brief Search result */
struct SearchResult {
    /** @brief Starting position of the first channel of the result in the dataset file */
    FilePositionT file_position;
    /** @brief Distance of the result to the query */
    DistanceT distance;

    /**
     * @brief Less than operator
     *
     * @param other The other SearchResult to compare to
     * @return `true` if the distance of this result is less than the distance of the other result
     */
    bool operator<(const SearchResult &other) const { return distance < other.distance; }
};

/** @brief Interface for result sets */
class IResultSet {
   public:
    virtual ~IResultSet() = default;

    /**
     * @brief Get the type of the result set
     *
     * @return The type of the result set
     */
    virtual SearchType get_type() const = 0;

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

    /** @brief Clear the result set */
    virtual void clear() = 0;
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

    SearchType get_type() const override;

    void insert(SearchResult result) override;

    vec<SearchResult> get_results() const override;

    DistanceT get_distance_lb() const override;

    void clear() override;

    /** @brief Get R */
    DistanceT get_r() const;

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
    KnnResultSet(uint k);

    SearchType get_type() const override;

    void insert(SearchResult result) override;

    vec<SearchResult> get_results() const override;

    DistanceT get_distance_lb() const override;

    void clear() override;

    /** @brief Get K */
    uint get_k() const;

   private:
    vec<SearchResult> m_results;
    uint m_k;
};

#endif  // RESULT_SET_HPP

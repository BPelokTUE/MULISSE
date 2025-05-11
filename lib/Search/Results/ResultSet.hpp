#ifndef RESULT_SET_HPP
#define RESULT_SET_HPP

#include "Enums/SearchType.hpp"
#include "Util/Types/Containers.hpp"
#include "Util/Types/Numbers.hpp"

template <SearchType S>
struct ResultSetParam;

template <>
struct ResultSetParam<KNN> {
    using ParamType = uint;
};

template <>
struct ResultSetParam<R_RANGE> {
    using ParamType = Real;
};

/**
 * @brief Template class for managing search results
 * @tparam S The type of search (KNN or R_RANGE)
 * */
template <SearchType S>
class ResultSet {
   public:
    using ParamType = typename ResultSetParam<S>::ParamType;

    /**
     * @brief Construct a new ResultSet object
     * @param param The parameter of the result set (either k for kNN or r for r-range)
     */
    ResultSet(ParamType param) : c_param(param) {};

    /**
     * @brief Get the type of the result set
     * @return The type of the result set
     */
    SearchType get_type() const { return S; }

    /**
     * @brief Insert a search result into the result set
     * @param result The search result to insert
     */
    void insert(SearchResult result) {
        if constexpr (S == KNN) {
            auto it = std::lower_bound(m_results.begin(), m_results.end(), result);
            m_results.insert(it, result);
            if (m_results.size() > c_param) m_results.pop_back();
        } else {  // R_RANGE
            if (result.m_distance <= c_param) m_results.push_back(result);
        }
    }

    /**
     * @brief Get the lower bound distance of the result set
     * @return The lower bound distance of the result set; No result with a greater distance should be considered
     */
    inline Real get_distance_lb() const {
        if constexpr (S == KNN) {
            return m_results.size() < c_param ? INF : m_results[c_param - 1].m_distance;
        } else {  // R_RANGE
            return c_param;
        }
    }

    /**
     * @brief Get the results in the result set
     * @return The results in the result set
     */
    const vec<SearchResult> &get_results() const { return m_results; }

    /** @brief Clear the result set */
    void clear() { m_results.clear(); }

    const ParamType c_param;

   private:
    vec<SearchResult> m_results;
};

#endif  // RESULT_SET_HPP

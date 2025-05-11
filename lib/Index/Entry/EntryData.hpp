#ifndef INDEX_ENTRY_ENTRYDATA_HPP
#define INDEX_ENTRY_ENTRYDATA_HPP

#include <type_traits>

#include "Util/Types/Containers.hpp"
#include "Util/Types/Numbers.hpp"

/** @brief Interface for data of IndexEntry objects */
class IEntryData {
   public:
    virtual ~IEntryData() = default;

    /**
     * @brief Get the size (number of entries) of the envelope
     * @return The size of the envelope
     * */
    virtual size_t size() const = 0;

    /**
     * @brief Resize the envelope
     * @param new_size The new size of the envelope
     * */
    virtual void resize(size_t new_size) = 0;

    /**
     * @brief Get the input for the iSAX index
     * @return The input for the iSAX index
     * */
    virtual const vec<Real> &get_isax_input() const = 0;
};

template <typename T>
concept DerivedFromEntryData = std::is_base_of_v<IEntryData, T>;

#endif  // INDEX_ENTRY_ENTRYDATA_HPP

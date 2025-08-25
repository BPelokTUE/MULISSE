#ifndef UTIL_TYPES_OUTPUTSTREAM_HPP
#define UTIL_TYPES_OUTPUTSTREAM_HPP

#include <ostream>

#include "Util/Types/Pointers.hpp"
#include "Util/Types/String.hpp"

class OutputStream {
    uptr<std::ostream> m_ostream;

    std::optional<str> m_path;

   public:
    /** @brief Default constructor */
    OutputStream() = default;

    /**
     * @brief Constructor that initializes the stream for writing to a file
     * @param path Path to the file to write to
     * @param open_mode Mode to open the file with (default: binary)
     */
    OutputStream(const str &path, std::ios_base::openmode open_mode = std::ios::binary);

    /**
     * @brief Check if the stream is set
     * @return True if the stream is set, false otherwise
     */
    explicit operator bool() const;

    /**
     * @brief Get the output stream
     * @return Reference to the output stream
     */
    std::ostream &get();

    /**
     * @brief Get the path of the output stream if it was initialized from a file
     * @return The path of the output stream
     */
    std::optional<str> get_path() const;
};

#endif  // UTIL_TYPES_OUTPUTSTREAM_HPP

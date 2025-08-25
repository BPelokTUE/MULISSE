#ifndef UTIL_TYPES_INPUTSTREAM_HPP
#define UTIL_TYPES_INPUTSTREAM_HPP

#include <istream>
#include <optional>

#include "Util/Types/Pointers.hpp"
#include "Util/Types/String.hpp"

class InputStream {
    uptr<std::istream> m_istream;

    std::optional<str> m_path;

   public:
    /** @brief Default constructor */
    InputStream() = default;

    /**
     * @brief Constructor that initializes the stream from a stream
     * @param istream Input stream to use
     */
    InputStream(uptr<std::istream> istream) {}

    /**
     * @brief Constructor that initializes the stream for writing to a file
     * @param path Path to the file to write to
     * @param open_mode Mode to open the file with (default: binary)
     */
    InputStream(const str &path, std::ios_base::openmode open_mode = std::ios::binary);

    /**
     * @brief Check if the stream is set
     * @return True if the stream is set, false otherwise
     */
    explicit operator bool() const;

    /**
     * @brief Get the output stream
     * @return Reference to the output stream
     */
    std::istream &get();

    /**
     * @brief Get the path of the input stream if it was initialized from a file
     * @return The path of the input stream
     */
    std::optional<str> get_path() const;
};

#endif  // UTIL_TYPES_INPUTSTREAM_HPP

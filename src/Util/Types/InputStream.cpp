#include "Util/Types/InputStream.hpp"

#include <fstream>

#include "Util/HelperFuncs/Path.hpp"

InputStream::InputStream(uptr<std::istream> istream) {
    if (!istream || !(*istream) || !istream->good()) {
        throw std::runtime_error("Failed to set input stream: stream is not valid.");
    }
    m_istream = std::move(istream);
}

InputStream::InputStream(const str &path, std::ios_base::openmode open_mode) {
    check_file_is_readable(path);
    m_istream = std::make_unique<std::ifstream>(path, open_mode);
    m_path = path;
}

InputStream::operator bool() const { return m_istream != nullptr; }

std::istream &InputStream::get() { return *m_istream; }

std::optional<str> InputStream::get_path() const { return m_path; }

#include "Util/Types/OutputStream.hpp"

#include <fstream>

#include "Util/HelperFuncs/Path.hpp"

OutputStream::OutputStream(const str &path, std::ios_base::openmode open_mode) {
    check_file_is_readable(path);
    m_ostream = std::make_unique<std::ofstream>(path, open_mode);
    m_path = path;
}

OutputStream::operator bool() const { return m_ostream != nullptr; }

std::optional<str> OutputStream::get_path() const { return m_path; }

std::ostream &OutputStream::get() { return *m_ostream; }

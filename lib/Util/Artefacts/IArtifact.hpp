#ifndef UTIL_ARTEFACTS_IARTIFACT_HPP
#define UTIL_ARTEFACTS_IARTIFACT_HPP

#include "Util/Types/String.hpp"

enum class ArchiveType;

class IArtifact {
   public:
    /**
     * @brief Save the artifact into a file
     * @param out_file Path to the output file
     * @param ar_type Archive type
     */
    virtual void save(const str &out_file, ArchiveType ar_type) = 0;

    /**
     * @brief Load the artifact from a file
     * @param in_file Path to the input file
     * @param ar_type Archive type
     */
    virtual void load(const str &in_file, ArchiveType ar_type) = 0;
};

#endif  // UTIL_ARTEFACTS_IARTIFACT_HPP

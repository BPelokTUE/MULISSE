#ifndef UTIL_ARTEFACTS_METAARTIFACT_HPP
#define UTIL_ARTEFACTS_METAARTIFACT_HPP

#include "Util/Artefacts/Artifact.hpp"

class MetaArtifact : public IArtifact {
   public:
    /**
     * @brief Save the meta artifact into a file
     * @param out_file Path to the output file
     */
    void save_meta(const str &out_file);

    /**
     * @brief Load the meta artifact from a file
     * @param in_file Path to the input file
     */
    void load_meta(const str &in_file);

    /**
     * @brief Get the path to the query_set meta file
     * @return The path to the query_set meta file
     */
    virtual str get_meta_path() const;
};

#endif  // UTIL_ARTEFACTS_METAARTIFACT_HPP

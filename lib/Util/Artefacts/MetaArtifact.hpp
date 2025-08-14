#ifndef UTIL_ARTEFACTS_METAARTIFACT_HPP
#define UTIL_ARTEFACTS_METAARTIFACT_HPP

#include "Util/Artefacts/Artifact.hpp"

class MetaArtifact : public IArtifact {
   public:
    void save_meta(const str &out_file);

    void load_meta(const str &in_file);
};

#endif  // UTIL_ARTEFACTS_METAARTIFACT_HPP

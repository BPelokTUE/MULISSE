#include "Util/Artefacts/MetaArtifact.hpp"

#include "Enums/ArchiveType.hpp"

void MetaArtifact::save_meta(const str &out_file) { save(out_file, ArchiveType::JSON); }

void MetaArtifact::load_meta(const str &in_file) { load(in_file, ArchiveType::JSON); }

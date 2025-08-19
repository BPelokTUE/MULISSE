#include "Util/Artefacts/MetaArtifact.hpp"

#include "Enums/ArchiveType.hpp"

void MetaArtifact::save_meta(const str &out_file) { save(out_file, ArchiveType::JSON); }

void MetaArtifact::load_meta(const str &in_file) { load(in_file, ArchiveType::JSON); }

void MetaArtifact::set_log_id(uint id) { m_log_id = id; }

uint MetaArtifact::get_log_id() const { return m_log_id; }

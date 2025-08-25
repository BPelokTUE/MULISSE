#include "Util/Artefacts/Artifact.hpp"

#include "Enums/ArchiveType.hpp"
#include "Util/Types/InputStream.hpp"
#include "Util/Types/OutputStream.hpp"

void IArtifact::save_meta(const str &out_file) {
    OutputStream o_stream(out_file);
    cereal::JSONOutputArchive ar(o_stream.get());
    apply_out_archive(ar);
}

void IArtifact::load_meta(const str &in_file) {
    InputStream i_stream(in_file);
    cereal::JSONInputArchive ar(i_stream.get());
    apply_in_archive(ar);
}

void IArtifact::set_log_id(uint id) { m_log_id = id; }

uint IArtifact::get_log_id() const { return m_log_id; }

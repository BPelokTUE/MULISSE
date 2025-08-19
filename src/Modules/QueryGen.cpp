#include "Modules/QueryGen.hpp"

#include "Util/Artefacts/MtsQuerySet.hpp"
#include "Util/Logging/QuerySetLogger.hpp"

void create_queries(MtsQuerySet &query_set, const QuerySetGenOptions &query_set_gen_opts, QuerySetLogger &logger) {
    query_set.generate(query_set_gen_opts);
    uint log_id = logger.write_entry(query_set, query_set_gen_opts);
    query_set.set_log_id(log_id);
}

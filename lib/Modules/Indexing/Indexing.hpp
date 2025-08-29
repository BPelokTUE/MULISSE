#ifndef MODULES_INDEXING_HPP
#define MODULES_INDEXING_HPP

class IndexManager;
struct IndexGenOptions;
class IndexLogger;

/**
 * @brief Create an index based on the provided general and specific properties and the generation options.
 * @param index_manager The index manager to use for creating the index
 * @param index_gen_opts Options for index generation
 * @param logger Logger to use for logging index creation details
 */
void create_index(IndexManager &index_manager, IndexGenOptions &index_gen_opts, IndexLogger &logger);

#endif  // MODULES_INDEXING_HPP

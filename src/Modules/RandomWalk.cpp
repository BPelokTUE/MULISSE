#include "Modules/RandomWalk.hpp"

#include <filesystem>
#include <fstream>
#include <random>
#include <sstream>

#include "Util/Artefacts/MtsDataset.hpp"
#include "Util/Artefacts/Options/RandomWalkGenOptions.hpp"
#include "Util/HelperFuncs/Conversion.hpp"
#include "Util/Logging/DatasetLogger.hpp"
#include "Util/Stats/ChannelStats.hpp"
#include "Util/Types/RunContext.hpp"

void create_random_walks(MtsDataset &dataset, const RandomWalkGenOptions &rw_gen_opts, DatasetLogger &logger) {
    dataset.generate_random_walks(rw_gen_opts);
    uint logger_id = logger.write_entry(rw_gen_opts, dataset);
    dataset.set_log_id(logger_id);
}

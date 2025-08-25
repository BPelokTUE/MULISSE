#include "Modules/CalcFfts.hpp"

#include "Util/Artefacts/MtsDataset.hpp"
#include "Util/Artefacts/MtsDatasetFfts.hpp"
#include "Util/Logging/FftsLogger.hpp"
#include "Util/Types/RunContext.hpp"

void calculate_ffts(MtsDatasetFfts &ffts, FftsLogger &logger) {
    logger.start_timer();
    ffts.generate();
    logger.stop_timer();
    logger.write_entry(ffts);
}

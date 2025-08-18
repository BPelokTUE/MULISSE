#include "Modules/CsvParsing.hpp"

#include <algorithm>
#include <iostream>
#include <random>

#include "Util/Artefacts/MtsDataset.hpp"
#include "Util/HelperFuncs/Conversion.hpp"
#include "Util/HelperFuncs/Math.hpp"
#include "Util/Logging/DatasetLogger.hpp"
#include "Util/Stats/ChannelStats.hpp"
#include "Util/Types/RunContext.hpp"

void create_dataset_from_csv(MtsDataset &dataset, const CsvDatasetGenOptions &csv_gen_opts,
                             const DatasetLogger &logger) {
    dataset.generate_from_csvs(csv_gen_opts);
    logger.write_entry(csv_gen_opts, dataset);
}

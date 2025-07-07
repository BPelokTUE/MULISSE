#ifndef MODULES_INDEXING_CONSTRUCTINDEX
#define MODULES_INDEXING_CONSTRUCTINDEX

#include <functional>

#include "Modules/Indexing/IndexFactory/IndexFactoryParams.hpp"
#include "Util/Types/Numbers.hpp"

template <typename T>
struct IIndex;

template <typename T>
class IEntryGenerator;

template <typename T>
class IEntryMerger;

class ILengthGroupSegmentationStrategy;

/**
 * @brief Constructs an index using the provided index factory
 * @param index_factory A function that creates an index based on the provided factory parameters
 * @param generator A unique pointer to an entry generator that produces entries for the index
 * @param merger A unique pointer to an entry merger that merges entries if applicable, nullptr otherwise
 * @param opts IndexOptions to use for constructing the index
 * @param lg_segmentation_strategy A unique pointer to a length group segmentation strategy that defines how to segment
 * @param sample_frac Fraction of the dataset to use for indexing, intended for testing, defaults to 1.0 (index the
 * whole dataset)
 */
template <typename T>
void construct_index(std::function<sptr<IIndex<T>>(IndexFactoryParams &)> index_factory,
                     uptr<IEntryGenerator<T>> generator, uptr<IEntryMerger<T>> merger, const IndexOptions &opts,
                     uptr<ILengthGroupSegmentationStrategy> lg_segmentation_strategy, Real sample_frac = 1.0);

template <typename T>
sptr<IIndex<T>> get_index_without_data(std::function<sptr<IIndex<T>>(IndexFactoryParams &)> index_factory,
                                       const IndexOptions &opts,
                                       const ILengthGroupSegmentationStrategy *lg_segmentation_strategy);

#endif  // MODULES_INDEXING_CONSTRUCTINDEX

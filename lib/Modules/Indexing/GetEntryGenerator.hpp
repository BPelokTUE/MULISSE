#ifndef MODULES_INDEXING_GETGENERATOR_HPP
#define MODULES_INDEXING_GETGENERATOR_HPP

#include "Index/Entry/Envelope.hpp"
#include "Index/Entry/Paa.hpp"
#include "Index/EntryGenerator/EntryGenerator.hpp"
#include "Index/IndexOptions.hpp"
#include "Index/Segmentation/LengthGroupSegmentationStrategy/LengthGroupSegmentationStrategy.hpp"

uptr<IEntryGenerator<Paa>> get_paa_generator(const IndexOptions &opts,
                                             const ILengthGroupSegmentationStrategy *lg_segmentation_strategy);

uptr<IEntryGenerator<Envelope>> get_envelope_generator(
    const IndexOptions &opts, const ILengthGroupSegmentationStrategy *lg_segmentation_strategy);

#endif  // MODULES_INDEXING_GETGENERATOR_HPP

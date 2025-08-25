#ifndef MODULES_INDEXING_INITIALIZEBREAKPOINTS_HPP
#define MODULES_INDEXING_INITIALIZEBREAKPOINTS_HPP

struct SaxProperties;

#include "Util/Types/Numbers.hpp"

/**
 * @brief Initializes SAX breakpoints based on the provided SaxParams.
 * @param sax_params The parameters containing the number of bits and breakpoint strategy.
 */
void initialize_sax_breakpoints(const SaxProperties &sax_params);

#endif  // MODULES_INDEXING_INITIALIZEBREAKPOINTS_HPP

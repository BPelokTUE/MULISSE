#include "Search/iSax/iSaxSplitStrategy.hpp"

SaxSplitIndT RoundRobinSplitStrategy::get_split_ind() { return m_current_split++; }

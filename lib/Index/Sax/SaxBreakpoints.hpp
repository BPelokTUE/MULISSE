#ifndef INDEX_SAX_SAXBREAKPOINTS_HPP
#define INDEX_SAX_SAXBREAKPOINTS_HPP

#include "Util/Types/Numbers.hpp"
#include "Util/Types/Vec.hpp"

class SaxBreakpoints {
    vec<Real> m_breakpoints;

   public:
    SaxBreakpoints(SaxSymbolT alphabet_size = 0);

    SaxBreakpoints(const vec<Real> &breakpoints);

    Real &operator[](size_t index);

    const Real &operator[](size_t index) const { return m_breakpoints[index]; }

    const size_t size() const;
};

#endif  // INDEX_SAX_SAXBREAKPOINTS_HPP

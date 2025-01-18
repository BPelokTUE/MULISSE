#include "typedefs.hpp"

class IiSaxSplitStrategy {
   public:
    virtual ~IiSaxSplitStrategy() {}
    virtual SaxSplitIndT get_split_ind() = 0;
};

class RoundRobinSplitStrategy : public IiSaxSplitStrategy {
   public:
    RoundRobinSplitStrategy(SaxNumBitsT num_bits) : m_num_bits(num_bits) {}

    SaxSplitIndT get_split_ind() override;

   private:
    SaxNumBitsT m_num_bits;
    SaxSplitIndT m_current_split = 0;
};

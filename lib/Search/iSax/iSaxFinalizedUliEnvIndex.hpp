#ifndef I_SAX_FINALIZED_ULI_ENV_INDEX_HPP
#define I_SAX_FINALIZED_ULI_ENV_INDEX_HPP

#include "Search/IUlisseEnvelopeIndex.hpp"

class iSaxFinalizedUliEnvIndex : public IFinalizedUliEnvIndex {
   public:
    iSaxFinalizedUliEnvIndex() = default;

    ~iSaxFinalizedUliEnvIndex() = default;

    void serialize(std::ofstream ofs) override;

    void deserialize(std::ifstream ifs) override;
};

#endif  // I_SAX_FINALIZED_ULI_ENV_INDEX_HPP

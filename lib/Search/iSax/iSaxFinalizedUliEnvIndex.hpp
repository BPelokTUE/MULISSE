#ifndef I_SAX_FINALIZED_ULI_ENV_INDEX_HPP
#define I_SAX_FINALIZED_ULI_ENV_INDEX_HPP

#include "Search/IUlisseEnvelopeIndex.hpp"
#include "Search/iSax/iSaxFinalizedNode.hpp"

class iSaxFinalizedUliEnvIndex : public IFinalizedUliEnvIndex {
   public:
    iSaxFinalizedUliEnvIndex() = default;

    iSaxFinalizedUliEnvIndex(vec<vec<iSaxWord>> first_isax_mins, vec<vec<iSaxWord>> first_isax_maxs,
                             vec<std::unique_ptr<iSaxFinalizedNode>> first_layer_nodes,
                             SaxNumBitsT first_layer_num_bits, SaxNumBitsT alphabet_num_bits, vec<float> breakpoints);

    ~iSaxFinalizedUliEnvIndex() = default;

    void serialize(std::ofstream ofs) override;

    void deserialize(std::ifstream ifs) override;

   private:
    vec<vec<iSaxWord>> m_first_isax_mins, m_first_isax_maxs;
    vec<std::unique_ptr<iSaxFinalizedNode>> m_first_layer_nodes;
    SaxNumBitsT m_first_layer_num_bits, m_alphabet_num_bits;
    SaxSegIndT m_num_seg_per_channel;
    MtsNumChannelsT m_num_channels;
    vec<float> m_breakpoints;
};

#endif  // I_SAX_FINALIZED_ULI_ENV_INDEX_HPP

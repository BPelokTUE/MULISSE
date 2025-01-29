#ifndef ENVELOPE_GENERATOR_HPP
#define ENVELOPE_GENERATOR_HPP

#include "typedefs.hpp"

#include "Search/IEnvelopeIndex.hpp"
#include "Search/IndexOptions.hpp"
#include "Summarization/UlisseEnvelope.hpp"

class IEnvelopeGenerator {
   public:
    virtual ~IEnvelopeGenerator() = default;
    virtual vec<EnvelopeEntry> get_entries(const vec<vec<float>> &mts) = 0;
};

class iSaxEnvelopeGenerator : public IEnvelopeGenerator {
   public:
    iSaxEnvelopeGenerator(const IndexOptions &opts);

    vec<EnvelopeEntry> get_entries(const vec<vec<float>> &mts) override;

   private:
    const IndexOptions &m_opts;
    UlisseEnvelopeParams m_uli_params;
    vec<Envelope> (*m_envelope_func)(const vec<float> &, const UlisseEnvelopeParams &);
};

#endif  // ENVELOPE_GENERATOR_HPP

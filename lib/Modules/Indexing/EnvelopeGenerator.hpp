#ifndef ENVELOPE_GENERATOR_HPP
#define ENVELOPE_GENERATOR_HPP

#include "typedefs.hpp"

#include "Search/IEnvelopeIndex.hpp"
#include "Search/IndexOptions.hpp"
#include "Summarization/UlisseEnvelope.hpp"

class IEnvelopeGenerator {
   public:
    virtual ~IEnvelopeGenerator() = default;
    virtual EnvelopeEntry generate_entry() = 0;
    virtual void set_mts(const vec<vec<float>> *mts) = 0;
};

class iSaxEnvelopeGenerator : public IEnvelopeGenerator {
   public:
    iSaxEnvelopeGenerator(const IndexOptions &opts);

    EnvelopeEntry generate_entry() override;

    void set_mts(const vec<vec<float>> *mts) override;

   private:
    const vec<vec<float>> *m_mts;
    const IndexOptions &m_opts;
    UlisseEnvelopeParams m_uli_params;
    Envelope (*m_envelope_func)(std::span<const float> const &, const UlisseEnvelopeParams &);

    FilePositionT m_position;
};

#endif  // ENVELOPE_GENERATOR_HPP

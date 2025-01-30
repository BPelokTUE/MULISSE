#ifndef ENVELOPE_GENERATOR_HPP
#define ENVELOPE_GENERATOR_HPP

#include "typedefs.hpp"

#include "Search/IEnvelopeIndex.hpp"
#include "Search/IndexOptions.hpp"
#include "Summarization/UlisseEnvelope.hpp"

/** @brief Interface for envelope generators */
class IEnvelopeGenerator {
   public:
    virtual ~IEnvelopeGenerator() = default;

    /**
     * @brief Generate envelopes for a given time series
     *
     * @param mts Multivariate time series
     * @param series_ind Index of the time series within the dataset
     * @return Envelope entries ()
     */
    virtual vec<EnvelopeEntry> get_entries(const vec<vec<float>> &mts, size_t series_ind) = 0;
};

/** @brief Envelope generator for iSAX (ULISSE) envelopes */
class iSaxEnvelopeGenerator : public IEnvelopeGenerator {
   public:
    /**
     * @brief Construct a new iSaxEnvelopeGenerator object
     *
     * @param opts Indexing options
     */
    iSaxEnvelopeGenerator(const IndexOptions &opts);

    /**
     * @brief Generate envelopes for a given time series
     *
     * @param mts Multivariate time series to get the envelope entries from
     * @param series_ind Index of the time series within the dataset
     */
    vec<EnvelopeEntry> get_entries(const vec<vec<float>> &mts, size_t series_ind) override;

   private:
    const IndexOptions &m_opts;
    UlisseEnvelopeParams m_uli_params;
    vec<Envelope> (*m_envelope_func)(const vec<float> &, const UlisseEnvelopeParams &);
};

#endif  // ENVELOPE_GENERATOR_HPP

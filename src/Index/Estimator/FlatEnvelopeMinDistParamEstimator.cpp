#include "Index/Estimator/Sampling/FlatEnvelopeMinDistParamEstimator.hpp"

#include "Index/Entry/Envelope.hpp"
#include "Index/Entry/IndexEntry.hpp"
#include "Index/IndexOptions.hpp"
#include "Index/Segmentation/LengthGroupSegmentationStrategy/LengthGroupSegmentationStrategy.hpp"
#include "Search/DistanceMeasure/EuclideanDistance.hpp"
#include "Util/HelperFuncs/Conversion.hpp"
#include "Util/HelperFuncs/Math.hpp"
#include "Util/RunSettings/LengthProperties.hpp"

FlatEnvelopeMinDistParamEstimator::FlatEnvelopeMinDistParamEstimator(const IndexOptions &index_opts)
    : FlatEnvelopeSamplingParamEstimator(index_opts) {
    m_query_accs.resize(index_opts.m_estimator_params->m_sampling_params->m_num_queries,
                        vec<vec<Real>>(index_opts.m_num_channels));
    estimate_params(index_opts);
}

void FlatEnvelopeMinDistParamEstimator::update_queries(std::stringstream &query_stream, uint num_queries) {
    query_stream.seekg(0);
    MtsNumChannelsT num_channels = static_cast<MtsNumChannelsT>(m_query_accs[0].size());

    for (uint q = 0; q < num_queries; ++q) {
        for (MtsNumChannelsT c = 0; c < num_channels; ++c) {
            Real query_sum = R(0.0), query_sq_sum = R(0.0);
            m_query_accs[q][c].push_back(R(0.0));

            str line;
            std::getline(query_stream, line);
            std::stringstream channel_stream(line);
            {
                Real value;
                while (channel_stream >> value) {
                    m_query_accs[q][c].push_back(value);
                    query_sum += value;
                    query_sq_sum += value * value;
                }
            }
            if (m_query_accs[q][c].empty()) continue;

            auto [mu, sigma] = calculate_mu_and_sigma(query_sum, query_sq_sum, U(m_query_accs[q][c].size()));
            for (uint i = 1; i < m_query_accs[q][c].size(); ++i) {
                m_query_accs[q][c][i] = m_query_accs[q][c][i - 1] + (m_query_accs[q][c][i] - mu) / sigma;
            }
            m_query_accs[q][c].push_back(m_query_accs[q][c].back());
        }
    }
}

void FlatEnvelopeMinDistParamEstimator::initialize_config_evaluation(const vec<FlatEnvelopeParams> &configurations) {
    m_max_min_dist_total = R(0.0);
    m_min_dist_totals.resize(configurations.size(), R(0.0));
}

bool FlatEnvelopeMinDistParamEstimator::is_config_better(
    uint config_ind, const vec<vec<IndexEntry<Envelope>>> entries, const IndexOptions &index_opts,
    const LengthProperties &length_props, const ILengthGroupSegmentationStrategy *lg_segmentation_strategy) {
    DistanceMeasure<KNN, ED> distance_measure(index_opts.m_normalized);

    MtsNumChannelsT num_channels = static_cast<MtsNumChannelsT>(m_query_accs[0].size());

    for (auto &query_acc : m_query_accs) {
        uint query_len = U(query_acc[0].size());  // TODO: fix this in case not all channels are used

        uint lg_ind = length_props.get_length_group(query_len);
        auto ch_segmentation_strategy = lg_segmentation_strategy->get_const_ch_segmentation_strategy(lg_ind);

        Real min_dist_sum_query = R(0.0);
        for (MtsNumChannelsT c = 0; c < num_channels; ++c) {
            auto segmentation_strategy = ch_segmentation_strategy->get_const_segmentation_strategy(c);
            vec<Real> query_ch_paa(segmentation_strategy->get_num_segments(query_len));

            uint seg_start = 0;
            for (SaxSegIndT seg_ind = 0; seg_ind < query_ch_paa.size(); ++seg_ind) {
                uint seg_len = segmentation_strategy->get_segment_len(seg_ind), seg_end = seg_start + seg_len;
                query_ch_paa[seg_ind] = (query_acc[c][seg_end + 1] - query_acc[c][seg_start + 1]) / R(seg_len);
                seg_start = seg_end;
            }

            for (auto &entry : entries[lg_ind]) {
                for (SaxSegIndT seg_ind = 0; seg_ind < query_ch_paa.size(); ++seg_ind) {
                    Real segment_len_r = R(segmentation_strategy->get_segment_len(seg_ind));
                    min_dist_sum_query += distance_measure.min_dist_squared(query_ch_paa[seg_ind],
                                                                            entry.m_mts_summary[c].m_lower[seg_ind],
                                                                            entry.m_mts_summary[c].m_upper[seg_ind]) *
                                          segment_len_r;
                }
            }
            m_min_dist_totals[config_ind] += min_dist_sum_query / R(entries[lg_ind].size());
        }
    }
    if (m_min_dist_totals[config_ind] > m_max_min_dist_total) {
        m_max_min_dist_total = m_min_dist_totals[config_ind];
        return true;
    }
    return false;
}

#ifndef UTIL_RUNSETTINGS_HPP
#define UTIL_RUNSETTINGS_HPP

#include <fstream>

#include "Enums/CommandType.hpp"
#include "Enums/SearchMethodType.hpp"
#include "Index/EnvelopeIndex/EnvelopeParams.hpp"
#include "Util/RunSettings/BreakpointProperties.hpp"
#include "Util/RunSettings/DatasetProperties.hpp"
#include "Util/RunSettings/EnvelopeProperties.hpp"
#include "Util/RunSettings/LengthProperties.hpp"
#include "Util/Types/FftArray.hpp"
#include "Util/Types/SubsequenceInfo.hpp"

struct EnvelopeParams;

class RunSettings {
   public:
    virtual ~RunSettings() = default;

    RunSettings(const RunSettings&) = delete;
    RunSettings& operator=(const RunSettings&) = delete;

    RunSettings();

    static void initialize(CommandType command_type, DatasetProperties dataset_props, LengthProperties length_props,
                           uint pos_per_env, const str& index_path, const str& ffts_path, const str& query_path,
                           SearchMethodType method_type, const str& logs_dir, const str& data_dir);

    static inline RunSettings& get_instance() {
        assert(initialized);
        return *instance.get();
    }

#ifdef ENABLE_TEST_CODE
    static void set_instance(sptr<RunSettings> instance);
#endif

    // ---------------------------------------------------- //
    // -------------------- SETTINGS ---------------------- //
    // ---------------------------------------------------- //

    // FFTs
    /** @brief Calculate and save the FFTs of the time series in the dataset */
    void calculate_ffts() const;

    /**
     * @brief Load the FFTs of the given time series in the dataset
     *
     * @param series_ind Index of the time series in the dataset
     * @param channel_ind Index of the channel in the time series
     * @return FFTs of the time series
     */
    FftArray get_ffts(uint series_ind, MtsNumChannelsT channel_ind);

    /**
     * @brief Check if the FFTs of the time series are supported
     *
     * @return True if the FFTs are supported, false otherwise
     */
    bool ffts_supported() const;

    /**
     * @brief Calculate and store the FFTs for the provided query
     *
     * @param q_channel Channel of the query to calculate FFTs for
     * @param channel_ind Index of the channel in the query
     */
    void calculate_query_ffts(const vec<MassT>& q_channel, MtsNumChannelsT channel_ind);

    /**
     * @brief Get the FFTs of the last query they were calculated for
     *
     * @param channel_ind Index of the channel in the query
     * @return Pointer to the FFTs of the last query
     */
    const FftArray* get_query_ffts(MtsNumChannelsT channel_ind) const;

    /** @brief Resets the FFTs of the query */
    void reset_query_ffts();

    /** @brief Get the size of the FFTs on disk */
    size_t get_ffts_size_on_disk();

    // iSAX

    /**
     * @brief Get the currently used iSAX interval breakpoints
     * @return The vector of breakpoints, excluding `-INF` and `INF` at the ends
     */
    virtual const vec<Real>& get_breakpoints();

    /** @brief Update the iSAX interval breakpoints */
    void update_breakpoints();

    /**
     * @brief Check if the iSAX breakpoints are set
     * @return True if the breakpoints are set, false otherwise
     */
    bool breakpoints_set() const;

    /**
     * @brief Set the iSAX properties for the run
     * @param breakpoint_props The breakpoint properties
     */
    void set_breakpoint_props(BreakpointProperties breakpoint_props);

    // Properties

    const DatasetProperties& get_dataset_props() const;

    // NOTE: virtual is needed for mocking with fakeit
    virtual const BreakpointProperties& get_breakpoint_props() const;

    const EnvelopeProperties& get_envelope_props() const;

    /**
     * @brief Set the positions per envelope for the run
     * @param pos_per_env The number of positions per envelope
     */
    void set_pos_per_env(uint pos_per_env);

    /**
     * @brief Set the flat envelope parameters (positions per envelope, lengths per group) for the run
     * @param flat_envelope_params The flat envelope parameters
     */
    void set_flat_envelope_params(const EnvelopeParams& flat_envelope_params);

    /**
     * @brief Get the length properties for the run. NOTE: this function is virtual for testing purposes.
     * @return The length properties
     */
    virtual const LengthProperties& get_length_props() const;

    void set_lengths_per_group(uint l_per_group);

    // Paths

    str get_dataset_path() const;

    str get_query_path() const;

    str get_index_path() const;

    str get_ffts_path() const;

    str get_logs_path() const;

    // ---------------------------------------------------- //

   private:
    // Command information
    CommandType m_command_type;

    // Dataset properties
    DatasetProperties m_dataset_props;

    // Length properties
    LengthProperties m_length_props;

    // Envelope properties
    EnvelopeProperties m_envelope_props;

    // Breakpoint properties
    uptr<ISaxBreakpointStrategy> m_breakpoint_strategy;
    BreakpointProperties m_breakpoint_props;
    bool m_breakpoints_props_set = false;

    // Index
    str m_index_file;

    // FFTs
    str m_ffts_file;
    std::ifstream m_ffts_ifs;
    vec<uptr<FftArray>> m_query_ffts;
    bool m_ffts_supported;

    // Query
    str m_query_file;

    // Static
    static sptr<RunSettings> instance;
    static bool initialized;

    // output directories
    str m_logs_dir, m_data_dir;

    // Friend classes
    friend class DatasetLogger;
    friend class QuerySetLogger;
    friend class QueryStatsLogger;
    friend class IndexLogger;
    friend class ParamEstimatesLogger;
    friend class IndexStatsLogger;
    friend class QueryLogger;
};

#endif  // UTIL_RUNSETTINGS_HPP

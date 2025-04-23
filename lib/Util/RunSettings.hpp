#ifndef RUN_SETTINGS_HPP
#define RUN_SETTINGS_HPP

#include <fstream>

#include "Search/Options/SearchMethodType.hpp"
#include "Summarization/iSaxBreakpointStrategy.hpp"
#include "Util/typedefs.hpp"
#include "Util/utilities.hpp"
#include "Util/FftArray.hpp"

/** @brief Enumeration type for the command type */
enum CommandType { CREATE_DS, PARSE_CSV, CREATE_QS, CALC_Q_STATS, INDEX, CALC_I_STATS, CALC_FFTS, SEARCH };

DEFINE_ENUM_CONSTS_NO_EXTRA(CommandType, CMD_TYPE, false);

struct DatasetProperties {
    MtsNumChannelsT m_num_channels;
    uint m_series_len;
    uint m_num_series;
    str m_file;
};

struct LengthProperties {
    uint m_l_min;
    uint m_l_max;
    uint m_l_per_group;
    uint m_num_l_groups;
};

struct EnvelopeProperties {
    uint m_pos_per_env;
    uint m_envs_per_ts;
};

struct BreakpointProperties {
    SaxNumBitsT m_breakpoint_num_bits;
    uptr<IiSaxBreakpointStrategy> m_breakpoint_strategy;
    vec<Real> m_breakpoints;
};

class RunSettings {
   public:
    virtual ~RunSettings() = default;

    RunSettings(const RunSettings&) = delete;
    RunSettings& operator=(const RunSettings&) = delete;

    RunSettings();

    static void initialize(CommandType command_type, DatasetProperties dataset_props, LengthProperties length_props,
                           uint pos_per_env, const str& index_path, const str& ffts_path, const str& query_path,
                           SearchMethodType method_type, const str& logs_dir);

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
     * @param subs_info Position within the dataset and length of the subsequence to load the FFTs for
     * @param channel_ind Index of the channel in the time series
     * @param num_components Number of FFT components to load
     * @return FFTs of the time series
     */
    FftArray get_ffts(SubsequenceInfo subs_info, MtsNumChannelsT channel_ind, uint num_components);

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
     * @param num_components Number of FFT components to load
     */
    void calculate_query_ffts(const vec<MassT>& q_channel, MtsNumChannelsT channel_ind, uint num_components);

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
     * @brief Set the iSAX properties for the run
     * @param breakpoint_props The breakpoint properties
     */
    void set_breakpoint_props(BreakpointProperties breakpoint_props);

    // Properties

    const DatasetProperties& get_dataset_props() const;

    const BreakpointProperties& get_breakpoint_props() const;

    const EnvelopeProperties& get_envelope_props() const;

    const LengthProperties& get_length_props() const;

    // Length properties

    /**
     * @brief Get the length group index the subsequence belongs to
     * @param subs_length Length of the subsequence
     * @return The index of the length group
     */
    inline uint get_length_group(uint subs_length) const {
        return (m_length_props.m_num_l_groups * (subs_length - m_length_props.m_l_min)) /
               (m_length_props.m_l_max - m_length_props.m_l_min + 1);  // CHECK
    }

    /**
     * @brief Get the maximum length of the specified length group
     * @param lg_ind Length group index
     * @return The maximum length of the length group
     */
    inline uint get_lg_l_min(uint lg_ind) const {
        return m_length_props.m_num_l_groups > 0
                   ? m_length_props.m_l_min +
                         (m_length_props.m_l_max - m_length_props.m_l_min) * lg_ind / m_length_props.m_num_l_groups
                   : m_length_props.m_l_min;
    }

    /**
     * @brief Get the minimum length of the specified length group
     * @param lg_ind Length group index
     * @return The minimum length of the length group
     */
    inline uint get_lg_l_max(uint lg_ind) const {
        return m_length_props.m_num_l_groups > 0 ? m_length_props.m_l_min +
                                                       (m_length_props.m_l_max - m_length_props.m_l_min) *
                                                           (lg_ind + 1) / m_length_props.m_num_l_groups -
                                                       (lg_ind != (m_length_props.m_num_l_groups - 1) ? 1 : 0)
                                                 : m_length_props.m_l_max;
    }

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
    uptr<IiSaxBreakpointStrategy> m_breakpoint_strategy;
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
    const str DATA_DIR = "../DATA/";
    str logs_dir;

    // Friend classes
    friend class DatasetLogger;
    friend class QueryLogger;
    friend class QueryStatsLogger;
    friend class IndexLogger;
    friend class IndexStatsLogger;
};

#endif  // RUN_SETTINGS_HPP

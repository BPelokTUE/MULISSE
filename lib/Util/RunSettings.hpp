#ifndef RUN_SETTINGS_HPP
#define RUN_SETTINGS_HPP

#include <fstream>

#include "Util/typedefs.hpp"
#include "Util/utilities.hpp"
#include "Util/FftArray.hpp"

/** @brief Enumeration type for the command type */
enum CommandType { CREATE_DS, PARSE_CSV, CREATE_QS, INDEX, SEARCH };

DEFINE_ENUM_CONSTS_NO_EXTRA(CommandType, CMD_TYPE, false);

struct DatasetProperties {
    str file;
    MtsNumChannelsT num_channels;
    uint series_len;
    uint num_series;
};

// TODO: Rewrite, `l_min` and `l_max` are not properties of the query
struct QueryProperties {
    str file;
    uint l_min;
    uint l_max;
};

struct EnvelopeProperties {
    uint pos_per_env;
    uint envs_per_ts;
};

struct iSaxProperties {
    SaxSegIndT num_segments;
    uint segment_len;
    vec<float> m_breakpoints;
    SaxNumBitsT m_breakpoint_num_bits;
};

class RunSettings {
   public:
    virtual ~RunSettings() = default;

    RunSettings(const RunSettings&) = delete;
    RunSettings& operator=(const RunSettings&) = delete;

    RunSettings();

    static void initialize(CommandType command_type, DatasetProperties dataset_props, QueryProperties query_props,
                           uint pos_per_env, const str index_path, const str ffts_path);

    static RunSettings& get_instance();

    static void set_instance(std::shared_ptr<RunSettings> instance);

    // ---------------------------------------------------- //
    // -------------------- SETTINGS ---------------------- //
    // ---------------------------------------------------- //

    // FFTs
    /** @brief Calculate and save the FFTs of the time series in the dataset */
    void calculate_ffts() const;

    /**
     * @brief Load the FFTs of the given time series in the dataset
     *
     * @param subs_pos Position within the dataset of the subsequence to load the FFTs for
     * @param channel_ind Index of the channel in the time series
     * @param num_components Number of FFT components to load
     * @return FFTs of the time series
     */
    FftArray get_ffts(SubsequencePosition subs_pos, MtsNumChannelsT channel_ind, uint num_components);

    /**
     * @brief Check if the FFTs of the time series are supported
     *
     * @return True if the FFTs are supported, false otherwise
     */
    bool ffts_supported() const;

    /**
     * @brief Calculate and store the FFTs for the provided query
     *
     * @param q_channel Channel of theQuery to calculate FFTs for
     * @param channel_ind Index of the channel in the query
     * @param num_components Number of FFT components to load
     */
    void calculate_query_ffts(const vec<DistanceT>& q_channel, MtsNumChannelsT channel_ind, uint num_components);

    /**
     * @brief Get the FFTs of the last query they were calculated for
     *
     * @param channel_ind Index of the channel in the query
     * @return Pointer to the FFTs of the last query
     */
    const FftArray* get_query_ffts(MtsNumChannelsT channel_ind) const;

    /** @brief Resets the FFTs of the query */
    void reset_query_ffts();

    // iSAX

    /**
     * @brief Get the currently used iSAX interval breakpoints
     * @return The vector of breakpoints, excluding `-INF` and `INF` at the ends
     */
    virtual const vec<float>& get_breakpoints();

    void set_isax_properties(iSaxProperties isax_props);

    // Properties

    const DatasetProperties& get_dataset_props();

    const QueryProperties& get_query_props();

    virtual const iSaxProperties& get_isax_props();

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

    // Query properties
    QueryProperties m_query_properties;

    // Envelope properties
    EnvelopeProperties m_envelope_props;

    // iSAX properties
    iSaxProperties m_isax_props;
    bool m_isax_props_set = false;

    // Index
    str m_index_file;

    // FFTs
    str m_ffts_file;
    std::ifstream m_ffts_ifs;
    vec<uptr<FftArray>> m_query_ffts;

    // Static
    static std::shared_ptr<RunSettings> instance;
    static bool initialized;

    // Constants
    const str DATA_DIR = "../DATA/", LOGS_DIR = "../LOGS/";

    // Friend classes
    friend class DatasetLogger;
    friend class IndexLogger;
    friend class QueryLogger;
};

#endif  // RUN_SETTINGS_HPP

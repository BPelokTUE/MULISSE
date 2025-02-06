#ifndef RUN_SETTINGS_HPP
#define RUN_SETTINGS_HPP

#include <fstream>

#include "Util/typedefs.hpp"
#include "Util/FftArray.hpp"

/** @brief Enumeration type for the command type */
enum CommandType { CREATE_DS, CREATE_QS, INDEX, SEARCH };

/** @brief Map from strings to CommandType */
const umap<str, CommandType> STR_TO_CMD_TYPE = {
    {"create_ds", CREATE_DS}, {"create_qs", CREATE_QS}, {"index", INDEX}, {"search", SEARCH}};

/** @brief Map from CommandType to strings */
const umap<CommandType, str> CMD_TYPE_TO_STR = get_inverse_map(STR_TO_CMD_TYPE);

struct DatasetProperties {
    str path;
    MtsNumChannelsT num_channels;
    unsigned series_len;
    unsigned num_series;
};

class RunSettings {
   public:
    RunSettings(const RunSettings&) = delete;
    RunSettings& operator=(const RunSettings&) = delete;

    RunSettings();

    static void initialize(CommandType command_type, DatasetProperties dataset_properties, str ffts_path);

    static RunSettings& get_instance();

    // ---------------------------------------------------- //
    // -------------------- SETTINGS ---------------------- //
    // ---------------------------------------------------- //

    // FFTs
    /** @brief Calculate and save the FFTs of the time series in the dataset */
    void calculate_ffts() const;

    /**
     * @brief Load the FFTs of the given time series in the dataset
     *
     * @param file_pos Starting position of the time series to get FFTs for
     * @param channel_ind Index of the channel in the time series
     * @param num_components Number of FFT components to load
     * @return FFTs of the time series
     */
    FftArray get_ffts(FilePositionT file_pos, MtsNumChannelsT channel_ind, unsigned num_components);

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
    void calculate_query_ffts(const vec<DistanceT>& q_channel, MtsNumChannelsT channel_ind, unsigned num_components);

    /**
     * @brief Get the FFTs of the last query they were calculated for
     *
     * @param channel_ind Index of the channel in the query
     * @return Pointer to the FFTs of the last query
     */
    const FftArray* get_query_ffts(MtsNumChannelsT channel_ind) const;

    /** @brief Resets the FFTs of the query */
    void reset_query_ffts();

    // ---------------------------------------------------- //

   private:
    // Command information
    CommandType m_command_type;

    // Dataset properties
    DatasetProperties m_dataset_props;

    // FFTs
    str m_ffts_path;
    std::ifstream m_ffts_stream;
    vec<uptr<FftArray>> m_query_ffts;

    static RunSettings instance;

    static bool initialized;
};

#endif  // RUN_SETTINGS_HPP

#ifndef RUN_SETTINGS_HPP
#define RUN_SETTINGS_HPP

#include "Util/typedefs.hpp"
#include "Util/FftArray.hpp"

struct RunSettings {
    RunSettings(const RunSettings&) = delete;
    RunSettings& operator=(const RunSettings&) = delete;

    static RunSettings& initialize() {
        static RunSettings instance;
        return instance;
    }

    // ---------------------------------------------------- //
    // -------------------- SETTINGS ---------------------- //
    // ---------------------------------------------------- //

    vec<FftArray> dataset_ffts;

    // ---------------------------------------------------- //

   private:
    RunSettings() = default;
};

#endif  // RUN_SETTINGS_HPP

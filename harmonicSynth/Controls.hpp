#ifndef CONTROLS_HPP_
#define CONTROLS_HPP_

#include <array>
#include <cstdint>

#include "Limit.hpp"

enum ControlPorts
{
    CONTROL_WAVEFORM   = 0,
    CONTROL_ATTACK     = 1,
    CONTROL_DECAY      = 2,
    CONTROL_SUSTAIN    = 3,
    CONTROL_RELEASE    = 4,
    CONTROL_LEVEL      = 5,
    CONTROL_CUTOFF_DIFF= 6,
    CONTROL_PEAK_PART  = 7,
    CONTROL_PEAK_HEIGHT= 8,
    CONTROL_WAVEFORM_2 = 9,
    CONTROL_WAVEFORM_2_MODE = 10,
    CONTROL_WAVEFORM_2_TARGET = 11,
    CONTROL_LEVEL_2    = 12,
    CONTROL_PITCH_2    = 13,
    CONTROL_ATTACK_2   = 14,
    CONTROL_DECAY_2    = 15,
    CONTROL_SUSTAIN_2  = 16,
    CONTROL_RELEASE_2  = 17,
    CONTROL_NR         = 18
};

constexpr std::array<std::pair<float, float>, CONTROL_NR> controlLimit =
{{
    {0.0f, 4.0f},
    {0.001f, 4.0f},
    {0.001f, 4.0f},
    {0.0f, 1.0f},
    {0.001f, 4.0f},
    {0.0f, 1.0f},
    {-10000.0f, 10000.0f},
    {0.0f, 30.0f},
    {1.0f, 8.0f},
    {0.0f, 4.0f},
    {0.0f, 1.0f},
    {0.0f, 1.0f},
    {0.0f, 1.0f},
    {0.0f, 127.0f},
    {0.001f, 4.0f},
    {0.001f, 4.0f},
    {0.001f, 1.0f},
    {0.001f, 4.0f}
}};

class Controls {
    public:

    private:
        std::array<const float*, CONTROL_NR> control_ptr;
        std::array<float, CONTROL_NR> control;

    public:
        Controls() :
            control_ptr {nullptr}
        {
            control_ptr.fill(nullptr);
            control.fill(0.0f);
        }
        void connectControlPort(const uint32_t port, void* data_location) {
            control_ptr[port] = static_cast<const float*>(data_location);
        }
        inline bool isEveryControlConnected() {

            for (int i=0; i<CONTROL_NR; ++i) {
                if (!control_ptr[i]) {
                    return false;
                }

            }
            return true;
        }

        inline bool updateValues() {
            /* copy and validate control port values */
            bool updated = false;
            for (int i=0; i<CONTROL_NR; ++i) {
                if (*control_ptr[i] != control[i]) {
                    control[i] = limit<float> (*control_ptr[i], controlLimit[i].first, controlLimit[i].second);
                    // if (i == CONTROL_LEVEL) controlLevel.set (control[i], 0.01 * rate);
                    if (i == CONTROL_CUTOFF_DIFF || i == CONTROL_PEAK_PART || i == CONTROL_PEAK_HEIGHT || i == CONTROL_WAVEFORM) {
                        updated = true;
                    }
                }
            }
            return updated;

            /* filter refreshing and control moving */
            // if (refresh_filter) {
            //     filter.setValues(
            //         control[CONTROL_CUTOFF_DIFF],
            //         control[CONTROL_PEAK_PART],
            //         control[CONTROL_PEAK_HEIGHT],
            //         static_cast<Waveform> (control[CONTROL_WAVEFORM])
            //     );
            // }
        }
        inline float get(ControlPorts index) {
            return control[index];
        }
        inline void set(ControlPorts index, float value) {
            control[index] = value;
        }
};

#endif
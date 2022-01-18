#ifndef CONTROLS_HPP_
#define CONTROLS_HPP_

#include <array>
#include <cstdint>

#include "Limit.hpp"
#include "Waveform.hpp"

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

enum Targets
{
    LEVEL_1,
    PEAK_PARTIAL_1
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
        std::array<float, CONTROL_NR> moddedControl;
        double position; // For waveform 2
        double rate;

    public:
        Controls(double rt) :
            control_ptr {nullptr},
            rate (rt)
        {
            control_ptr.fill(nullptr);
            control.fill(0.0f);
            moddedControl.fill(0.0f);
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
            return getAbsolute(index) + getModded(index);
        }
        inline void set(ControlPorts index, float value) {
            control[index] = value;
        }
        inline float getModded(ControlPorts index) {
            return moddedControl[index];
        }
        inline float getAbsolute(ControlPorts index) {
            return control[index];
        }
        inline double proceed() {
            float freq = control[CONTROL_PITCH_2];
            position += freq / rate;
            if (control[CONTROL_WAVEFORM_2] == WAVEFORM_SQUARE) {
                if (control[CONTROL_WAVEFORM_2_TARGET] == LEVEL_1) {
                    float p = fmod(position, 1.0f);
                    float val = (p < 0.5f ? 1.0f : -1.0f);
                    val *= control[CONTROL_LEVEL_2];
                    moddedControl[CONTROL_LEVEL] = val;
                    // std::cout << "Mod control: " << val << std::endl;
                }
            }
            return position;
        }
};

#endif
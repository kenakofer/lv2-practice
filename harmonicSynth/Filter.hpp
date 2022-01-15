
#ifndef FILTER_HPP_
#define FILTER_HPP_

#include "Waveform.hpp"

const float KEY_TRACK_FREQUENCY = 440.0f;
const int CACHED_WAVE_SAMPLES = 512;
class Filter
{
    private:
        float cutoff_diff;
        float peak_freq;
        float peak_height;
        Waveform waveform;

        float cutoff_freq;
        float flat_freq;
        float slope;

        std::array<float, CACHED_WAVE_SAMPLES> cachedValues;

    public:
        Filter();
        float attenuationForFreq(float freq);
        void setValues(float cf, float pf, float ph, Waveform wf);
        float valueInWave(float pos);

    private:
        void _recalculate_values();
        void _refillCache();
};

inline Filter::Filter () :
    peak_height (1.2f),
    peak_freq (5000.0f),
    cutoff_diff (1000.0f)
{
    cachedValues.fill(0.0f);
    _recalculate_values();
}

inline float Filter::attenuationForFreq(float freq) {
    if (slope > 0) {
        // Hi pass
        if (freq < cutoff_freq) return 0.0f;
        if (freq < peak_freq) return slope * (freq - cutoff_freq);
        if (freq < flat_freq) return peak_height - (slope * (freq - peak_freq));
        else return 1.0f;
    } else {
        // Lo pass
        if (freq < flat_freq) return 1.0f;
        if (freq < peak_freq) return peak_height - (slope * (freq - peak_freq));
        if (freq < cutoff_freq) return slope * (freq - cutoff_freq);
        else return 0.0f;
    }
}
inline void Filter::setValues(float cd, float pf, float ph, Waveform wf) {
    cutoff_diff = cd;
    peak_freq = pf;
    peak_height = ph;
    waveform = wf;
    _recalculate_values();
}

inline void Filter::_recalculate_values() {
    cutoff_freq = peak_freq + cutoff_diff;
    if (cutoff_freq < 20.0f) {
        cutoff_freq = 20.0f;
    } else if (cutoff_freq > 20000.0f) {
        cutoff_freq == 20000.0f;
    }
    slope = peak_height / (peak_freq - cutoff_freq);
    flat_freq = peak_freq + (peak_height - 1.0f) / slope;

    std::cout << "New filter settings for an A440 note:" << std::endl;
    std::cout << "Cutoff freq: " << cutoff_freq << std::endl;
    std::cout << "Peak freq: " << peak_freq << std::endl;
    std::cout << "Peak height: " << peak_height << std::endl;

    _refillCache();
}

inline void Filter::_refillCache() {
    float value;
    for (int n=0; n<CACHED_WAVE_SAMPLES; n++) {
        value = 0.0f;
        for (int i=0; i<PARTIAL_NR; i++) {
            if (PARTIAL_AMPLITUDES[waveform][i] == 0.0f) continue;
            value += PARTIAL_AMPLITUDES[waveform][i] *
                    attenuationForFreq(KEY_TRACK_FREQUENCY * (i+1)) *
                    sin(2.0 * M_PI * (i+1) * ((float)n/CACHED_WAVE_SAMPLES));
        }
        cachedValues[n] = value;
    }
}

inline float Filter::valueInWave(float pos) {
    return cachedValues[(int)(CACHED_WAVE_SAMPLES * fmod(pos, 1.0))];
}

#endif
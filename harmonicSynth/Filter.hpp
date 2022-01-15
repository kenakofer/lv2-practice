
#ifndef FILTER_HPP_
#define FILTER_HPP_

#include "Waveform.hpp"

const float KEY_TRACK_FREQUENCY = 440.0f;
const int CACHED_WAVE_SAMPLES = 512;


// 10 partials becomes unsafe/unnecessary above 2205 hz, or A4 + 28 semitones == c# == midi note 97
// 20 partials becomes unsafe above 1103 hz, or A4 + 15 semitones == c == midi note 84
// 30 partials becomes unsafe above 735 hz, or A4 + 7 semitones == e == midi note 76

// Unsafe, because trying to render frequencies above the nyquil limit (rate/2) actually reflects the frequency downward, ghosting unwanted frequencies.
// Unnecessary, because even if it did render correctly, it wouldn't be audible.

const float PARTIAL_LIMIT_10 = 20000.0f / 10;
const float PARTIAL_LIMIT_20 = 20000.0f / 20;
const float PARTIAL_LIMIT_30 = 20000.0f / 30;

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

        std::array<float, CACHED_WAVE_SAMPLES> cachedValues06;
        std::array<float, CACHED_WAVE_SAMPLES> cachedValues10;
        std::array<float, CACHED_WAVE_SAMPLES> cachedValues20;
        std::array<float, CACHED_WAVE_SAMPLES> cachedValues30;
        std::array<float, CACHED_WAVE_SAMPLES> cachedValues40;

    public:
        Filter();
        float attenuationForFreq(float freq);
        void setValues(float cf, float pf, float ph, Waveform wf);
        float valueInWave(float freq, float pos);

    private:
        void _recalculate_values();
        void _refillCache();
};

inline Filter::Filter () :
    peak_height (1.2f),
    peak_freq (5000.0f),
    cutoff_diff (1000.0f)
{
    cachedValues10.fill(0.0f);
    cachedValues20.fill(0.0f);
    cachedValues30.fill(0.0f);
    cachedValues40.fill(0.0f);
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

    // std::cout << "New filter settings for an A440 note:" << std::endl;
    // std::cout << "Cutoff freq: " << cutoff_freq << std::endl;
    // std::cout << "Peak freq: " << peak_freq << std::endl;
    // std::cout << "Peak height: " << peak_height << std::endl;

    _refillCache();
}

inline void Filter::_refillCache() {
    float value;
    for (int n=0; n<CACHED_WAVE_SAMPLES; n++) {
        value = 0.0f;
        for (int i=0; i<6; i++) {
            if (PARTIAL_AMPLITUDES[waveform][i] == 0.0f) continue;
            value += PARTIAL_AMPLITUDES[waveform][i] *
                    attenuationForFreq(KEY_TRACK_FREQUENCY * (i+1)) *
                    sin(2.0 * M_PI * (i+1) * ((float)n/CACHED_WAVE_SAMPLES));

        }
        cachedValues06[n] = value;
        for (int i=6; i<10; i++) {
            if (PARTIAL_AMPLITUDES[waveform][i] == 0.0f) continue;
            value += PARTIAL_AMPLITUDES[waveform][i] *
                    attenuationForFreq(KEY_TRACK_FREQUENCY * (i+1)) *
                    sin(2.0 * M_PI * (i+1) * ((float)n/CACHED_WAVE_SAMPLES));

        }
        cachedValues10[n] = value;
        for (int i=10; i<20; i++) {
            if (PARTIAL_AMPLITUDES[waveform][i] == 0.0f) continue;
            value += PARTIAL_AMPLITUDES[waveform][i] *
                    attenuationForFreq(KEY_TRACK_FREQUENCY * (i+1)) *
                    sin(2.0 * M_PI * (i+1) * ((float)n/CACHED_WAVE_SAMPLES));

        }
        cachedValues20[n] = value;
        for (int i=20; i<30; i++) {
            if (PARTIAL_AMPLITUDES[waveform][i] == 0.0f) continue;
            value += PARTIAL_AMPLITUDES[waveform][i] *
                    attenuationForFreq(KEY_TRACK_FREQUENCY * (i+1)) *
                    sin(2.0 * M_PI * (i+1) * ((float)n/CACHED_WAVE_SAMPLES));

        }
        cachedValues30[n] = value;
        // for (int i=30; i<40; i++) {
        //     if (PARTIAL_AMPLITUDES[waveform][i] == 0.0f) continue;
        //     value += PARTIAL_AMPLITUDES[waveform][i] *
        //             attenuationForFreq(KEY_TRACK_FREQUENCY * (i+1)) *
        //             sin(2.0 * M_PI * (i+1) * ((float)n/CACHED_WAVE_SAMPLES));

        // }
        // cachedValues40[n] = value;
    }
}

inline float Filter::valueInWave(float freq, float pos) {
    int index = (int)(CACHED_WAVE_SAMPLES * fmod(pos, 1.0));
    if (freq > PARTIAL_LIMIT_10) return cachedValues06[index];
    if (freq > PARTIAL_LIMIT_20) return cachedValues10[index];
    if (freq > PARTIAL_LIMIT_30) return cachedValues20[index];
    // if (freq > PARTIAL_LIMIT_40) return cachedValues30[index];
    return cachedValues30[index];
}

#endif

#ifndef FILTER2_HPP_
#define FILTER2_HPP_

#include <math.h>
#include "Waveform.hpp"
#include "whiteband500to1k.hpp"
#include "Controls.hpp"

const int CACHED_WAVE_SAMPLES = 512;


// 10 partials becomes unsafe/unnecessary above 2205 hz, or A4 + 28 semitones == c# == midi note 97
// 20 partials becomes unsafe above 1103 hz, or A4 + 15 semitones == c == midi note 84
// 30 partials becomes unsafe above 735 hz, or A4 + 7 semitones == e == midi note 76

// Unsafe, because trying to render frequencies above the nyquil limit (rate/2) actually reflects the frequency downward, ghosting unwanted frequencies.
// Unnecessary, because even if it did render correctly, it wouldn't be audible.

const float PARTIAL_LIMIT_10 = 22000.0f / 10;
const float PARTIAL_LIMIT_20 = 22000.0f / 20;
const float PARTIAL_LIMIT_30 = 22000.0f / 30;
const float PARTIAL_LIMIT_40 = 22000.0f / 40;
const float PARTIAL_LIMIT_50 = 22000.0f / 50;

class Filter
{
private:
    Controls* controls;
    Waveform waveform;

    std::array<float, CACHED_WAVE_SAMPLES> cachedValues01;
    std::array<float, CACHED_WAVE_SAMPLES> cachedValues06;
    std::array<float, CACHED_WAVE_SAMPLES> cachedValues10;
    std::array<float, CACHED_WAVE_SAMPLES> cachedValues20;
    std::array<float, CACHED_WAVE_SAMPLES> cachedValues30;
    std::array<float, CACHED_WAVE_SAMPLES> cachedValues40;
    std::array<float, CACHED_WAVE_SAMPLES> cachedValues50;
    // std::array<float, CACHED_WAVE_SAMPLES> cachedValues40;

public:
    inline Filter()
    {
        cachedValues01.fill(0.0f);
        cachedValues06.fill(0.0f);
        cachedValues10.fill(0.0f);
        cachedValues20.fill(0.0f);
        cachedValues30.fill(0.0f);
        cachedValues40.fill(0.0f);
        cachedValues50.fill(0.0f);
    }


    inline void recalculateValues(Controls *c) {
        controls = c;
        fillCache();
    }

    inline void fillCache() {
        waveform = static_cast<Waveform>((*controls).get(CONTROL_WAVEFORM));

        float value;
        for (int n=0; n<CACHED_WAVE_SAMPLES; n++) {
            value = 0.0f;
            for (int i=0; i<1; i++) {
                if (PARTIAL_AMPLITUDES[waveform][i] == 0.0f) continue;
                value += PARTIAL_AMPLITUDES[waveform][i] *
                        sin(2.0 * M_PI * (i+1) * ((float)n/CACHED_WAVE_SAMPLES));

            }
            cachedValues01[n] = value;
            for (int i=1; i<6; i++) {
                if (PARTIAL_AMPLITUDES[waveform][i] == 0.0f) continue;
                value += PARTIAL_AMPLITUDES[waveform][i] *
                        sin(2.0 * M_PI * (i+1) * ((float)n/CACHED_WAVE_SAMPLES));

            }
            cachedValues06[n] = value;
            for (int i=6; i<10; i++) {
                if (PARTIAL_AMPLITUDES[waveform][i] == 0.0f) continue;
                value += PARTIAL_AMPLITUDES[waveform][i] *
                        sin(2.0 * M_PI * (i+1) * ((float)n/CACHED_WAVE_SAMPLES));

            }
            cachedValues10[n] = value;
            for (int i=10; i<20; i++) {
                if (PARTIAL_AMPLITUDES[waveform][i] == 0.0f) continue;
                value += PARTIAL_AMPLITUDES[waveform][i] *
                        sin(2.0 * M_PI * (i+1) * ((float)n/CACHED_WAVE_SAMPLES));

            }
            cachedValues20[n] = value;
            for (int i=20; i<30; i++) {
                if (PARTIAL_AMPLITUDES[waveform][i] == 0.0f) continue;
                value += PARTIAL_AMPLITUDES[waveform][i] *
                        sin(2.0 * M_PI * (i+1) * ((float)n/CACHED_WAVE_SAMPLES));

            }
            cachedValues30[n] = value;
            for (int i=30; i<40; i++) {
                if (PARTIAL_AMPLITUDES[waveform][i] == 0.0f) continue;
                value += PARTIAL_AMPLITUDES[waveform][i] *
                        sin(2.0 * M_PI * (i+1) * ((float)n/CACHED_WAVE_SAMPLES));

            }
            cachedValues40[n] = value;
            for (int i=40; i<50; i++) {
                if (PARTIAL_AMPLITUDES[waveform][i] == 0.0f) continue;
                value += PARTIAL_AMPLITUDES[waveform][i] *
                        sin(2.0 * M_PI * (i+1) * ((float)n/CACHED_WAVE_SAMPLES));

            }
            cachedValues50[n] = value;
        }
    }

    inline float valueInWave(float freq, float position, float partials) {
        if (waveform == WAVEFORM_NOISE) return valueInNoise(freq, position, partials);

        float pos = fmod(position, 1.0);
        int i = (int)(CACHED_WAVE_SAMPLES * pos);

        if (freq > PARTIAL_LIMIT_10 && partials > 6.0f) return cachedValues06[i];
        if (freq > PARTIAL_LIMIT_20 && partials > 10.0f) return cachedValues10[i];
        if (freq > PARTIAL_LIMIT_30 && partials > 20.0f) return cachedValues20[i];
        if (freq > PARTIAL_LIMIT_40 && partials > 30.0f) return cachedValues30[i];
        if (freq > PARTIAL_LIMIT_50 && partials > 40.0f) return cachedValues40[i];

        if (partials < 1) {
            return partials * cachedValues01[i];
        } else if (partials < 6) {
            float p2 = (partials - 1) / 5;
            float p1 = 1.0f - p2;
            return (p1 * cachedValues01[i]) + (p2 * cachedValues06[i]);
        } else if (partials < 10) {
            float p2 = (partials - 6) / 4;
            float p1 = 1.0f - p2;
            return (p1 * cachedValues06[i]) + (p2 * cachedValues10[i]);
        } else if (partials < 20) {
            float p2 = (partials - 10) / 10;
            float p1 = 1.0f - p2;
            return (p1 * cachedValues10[i]) + (p2 * cachedValues20[i]);
        } else if (partials < 30) {
            float p2 = (partials - 20) / 10;
            float p1 = 1.0f - p2;
            return (p1 * cachedValues20[i]) + (p2 * cachedValues30[i]);
        } else if (partials < 40) {
            float p2 = (partials - 30) / 10;
            float p1 = 1.0f - p2;
            return (p1 * cachedValues30[i]) + (p2 * cachedValues40[i]);
        } else if (partials < 50) {
            float p2 = (partials - 40) / 10;
            float p1 = 1.0f - p2;
            return (p1 * cachedValues40[i]) + (p2 * cachedValues50[i]);
        } else {
            float p2 = (partials - 50) / 10;
            float p1 = 1.0f - p2;
            return (p1 * cachedValues50[i]) + (p2 * valueInWaveform(waveform, pos)); //Average with the perfect waveform type
        }
    }

    inline float valueInNoise(float freq, float position, float partials) {
        float samples_per_cycle = 44100.0f / 750;

        float value = 0.0f;
        float i=0.0f;
        while (i < 20) {
            if ((freq * (i+1)) > (44100 / 2)) break;
            if (i >= partials) break;

            int index = ((int)(samples_per_cycle * position * (i+1))) % WHITEBAND500TO1K_LENGTH;
            float factor = 1.0;
            if ((partials - 1 < (i-1)) && ((i-1) < partials)) {
                factor = 1.0f - (partials - (i-1));
            }
            value += WHITEBAND500TO1K_SAMPLES[index] * factor;
            if (i==0.0f) i += .85f;
            else i += .95;
        }
        return value;
    }

};

#endif
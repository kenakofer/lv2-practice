#ifndef KEY_HPP_
#define KEY_HPP_

#include <cstdint>
#include <cmath>
#include <random>
#include <ctime>
#include <array>
#include "LinearFader.hpp"
#include "Waveform.hpp"
#include "KeyStatus.hpp"
#include "Sine.hpp"
#include "Filter2.hpp"
#include "Controls.hpp"

class Key
{
private:
    KeyStatus status;
    Waveform waveform;
    uint8_t note;
    uint8_t velocity;
    double rate;
    double position;
    float start_level_1;
    float start_level_2;
    double freq;
    double time;
    LinearFader<float> fader;
    std::minstd_rand rnd;
    std::uniform_real_distribution<float> dist;
    Controls* controls;
    Filter* filter;

public:
    Key ();
    Key (const double rt);
    void press (const uint8_t nt, const uint8_t vel, Controls *c, Filter *f);
    void release ();
    void release (const uint8_t nt, const uint8_t vel);
    void off ();
    void mute ();
    float get ();
    void proceed ();
    bool isOn();

private:
    float adsr (int whichOscillator);
    float synth ();
    float synthPartials ();
    void setCachedValue(double pos, float val);
    float getCachedValue(double pos);
};

inline Key::Key () :
    Key (44100)
{

}

inline Key::Key (const double rt) :
    status (KEY_OFF),
    waveform (WAVEFORM_SINE),
    note (0),
    velocity (0),
    rate (rt),
    position (0.0),
    start_level_1 (0.0f),
    start_level_2 (0.0f),
    freq (pow (2.0, (static_cast<double> (note) - 69.0) / 12.0) * 440.0),
    time (0.0),
    fader (1.0f),
    rnd (std::time (0)),
    dist (-1.0f, 1.0f),
    filter ()
{

}

inline void Key::press (const uint8_t nt, const uint8_t vel, Controls *c, Filter *f)
{

    controls = c;
    start_level_1 = adsr(1);
    start_level_2 = adsr(2);
    note = nt;
    velocity = vel;
    freq = pow (2.0, (static_cast<double> (note) - 69.0) / 12.0) * 440.0;
    time = 0.0;
    fader.set (1.0f, 0.0);
    status = KEY_PRESSED;
    filter = f;

    // std::cout << "Starting note with freq: " << freq << std::endl;
}

inline void Key::release ()
{
    release (note, velocity);
}

inline void Key::release (const uint8_t nt, const uint8_t vel)
{
    if ((status == KEY_PRESSED) && (note == nt))
    {
        start_level_1 = adsr (1);
        start_level_2 = adsr (2);
        time = 0.0;
        status = KEY_RELEASED;
    }
}

inline void Key::off ()
{
    position = 0.0;
    status = KEY_OFF;
}

inline void Key::mute ()
{
    fader.set (0.0f, 0.01 * rate);
}

inline float Key::adsr(int whichEnvelope)
{
    float attack;
    float decay;
    float sustain;
    float release;
    float start_level;
    switch (whichEnvelope) {
        case (1):
            attack = (*controls).get(CONTROL_ATTACK);
            decay = (*controls).get(CONTROL_DECAY);
            sustain = (*controls).get(CONTROL_SUSTAIN);
            release = (*controls).get(CONTROL_RELEASE);
            start_level = start_level_1;
            break;
        case (2):
            attack = (*controls).get(CONTROL_ATTACK_2);
            decay = (*controls).get(CONTROL_DECAY_2);
            sustain = (*controls).get(CONTROL_SUSTAIN_2);
            release = (*controls).get(CONTROL_RELEASE_2);
            start_level = start_level_2;
            break;
        default:
            std::cerr << "Error: Envelope #" << whichEnvelope << " not found." << std::endl;
    }


    switch (status)
    {
    case KEY_PRESSED:
        if (time < attack)
        {
            return start_level + (1.0f - start_level) * time /attack;
        }

        if (time < attack + decay)
        {
            return 1.0f + (sustain - 1.0f) * (time - attack) / decay;
        }

        return sustain;

    case KEY_RELEASED:
        return start_level - start_level * time / release;

    default:
        return 0.0f;
    }
}

inline float Key::synthPartials()
{
    // return (*filter).valueInWave(freq, position);

    float peak_part = (*controls).get(CONTROL_PEAK_PART);
    if ((*controls).get(CONTROL_ENV_MODE_1) == ENV_CUTOFF_1) {
        peak_part += 4 * adsr(1);
    }
    if ((*controls).get(CONTROL_ENV_MODE_2) == ENV_CUTOFF_1) {
        peak_part += 4 * adsr(2);
    }
    return (*filter).valueInWave(freq, position, peak_part);

}

inline float Key::synth()
{
    const float p = fmod (position, 1.0);

    switch (static_cast<Waveform> ((*controls).get(CONTROL_WAVEFORM)))
    {
        case WAVEFORM_SINE:     return sin (2.0 * M_PI * position);
        case WAVEFORM_TRIANGLE: return (p < 0.25f ? 4.0f * p : (p < 0.75f ? 1.0f - 4.0 * (p - 0.25f) : -1.0f + 4.0f * (p - 0.75f)));
        case WAVEFORM_SQUARE:   return (p < 0.5f ? 1.0f : -1.0f);
        case WAVEFORM_SAW:      return 2.0f * p - 1.0f;
        default:                return 0.0f;
    }
}

inline float Key::get ()
{
    float value = synthPartials() *
                    (static_cast<float> (velocity) / 127.0f) *
                    fader.get();
    if ((*controls).get(CONTROL_ENV_MODE_1) == ENV_LEVEL_1) {
        value *= adsr(1);
    }
    if ((*controls).get(CONTROL_ENV_MODE_2) == ENV_LEVEL_1) {
        value *= adsr(2);
    }

    return value;
}

inline void Key::proceed ()
{
    float modfreq = freq;
    modfreq *= pow (2.0, (*controls).get(CONTROL_PITCH) / 12.0);
    if ((*controls).get(CONTROL_ENV_MODE_1) == ENV_PITCH_1) {
        modfreq *= pow (2.0, adsr(1) * 12 / 12.0); // Scale by 12 so it's an octave, TODO make more flexible
    }
    if ((*controls).get(CONTROL_ENV_MODE_2) == ENV_PITCH_1) {
        modfreq *= pow (2.0, adsr(2) * 12 / 12.0); // Scale by 12 so it's an octave, TODO make more flexible
    }
    time += 1.0 / rate;
    position += modfreq / rate;
    if ((status == KEY_RELEASED) && (time >= (*controls).get(CONTROL_RELEASE))) off();
}

inline bool Key::isOn () {
    return (status != KEY_OFF);
}

#endif /* KEY_HPP_ */
#ifndef KEY_HPP_
#define KEY_HPP_

#include <cstdint>
#include <cmath>
#include <random>
#include <ctime>
#include <array>
#include "LinearFader.hpp"
#include "Waveform.hpp"
#include "Envelope.hpp"
#include "KeyStatus.hpp"
#include "Sine.hpp"
#include "Filter.hpp"

class Key
{
private:
    KeyStatus status;
    Waveform waveform;
    uint8_t note;
    uint8_t velocity;
    Envelope envelope;
    double rate;
    double position;
    float start_level;
    double freq;
    double time;
    LinearFader<float> fader;
    std::minstd_rand rnd;
    std::uniform_real_distribution<float> dist;
    std::array<Sine, PARTIAL_NR> partials;
    Filter filter;

public:
    Key ();
    Key (const double rt);
    void press (const Waveform wf, const uint8_t nt, const uint8_t vel, const Envelope env, const Filter f);
    void release ();
    void release (const uint8_t nt, const uint8_t vel);
    void off ();
    void mute ();
    float get ();
    void proceed ();
    bool isOn();

private:
    float adsr ();
    float synth ();
    float synthPartials ();
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
    envelope {0.0, 0.0, 0.0f, 0.0},
    rate (rt),
    position (0.0),
    start_level (0.0f),
    freq (pow (2.0, (static_cast<double> (note) - 69.0) / 12.0) * 440.0),
    time (0.0),
    fader (1.0f),
    rnd (std::time (0)),
    dist (-1.0f, 1.0f),
    partials (),
    filter ()
{
    for (int i=0; i<PARTIAL_NR; i++) {
        partials[i].amplitude = PARTIAL_AMPLITUDES[waveform][i];
        partials[i].freq = freq * (i + 1);
        partials[i].attenuation = 1.0f;
    }
}

inline void Key::press (const Waveform wf, const uint8_t nt, const uint8_t vel, const Envelope env, const Filter f)
{
    start_level = adsr();
    note = nt;
    velocity = vel;
    envelope = env;
    freq = pow (2.0, (static_cast<double> (note) - 69.0) / 12.0) * 440.0;
    time = 0.0;
    fader.set (1.0f, 0.0);
    waveform = wf;
    status = KEY_PRESSED;
    filter = f;
    for (int i=0; i<PARTIAL_NR; i++) {
        partials[i].amplitude = PARTIAL_AMPLITUDES[waveform][i];
        partials[i].freq = freq * (i + 1);
        partials[i].attenuation = filter.attenuationForFreq(partials[i].freq);
    }
}

inline void Key::release ()
{
    release (note, velocity);
}

inline void Key::release (const uint8_t nt, const uint8_t vel)
{
    if ((status == KEY_PRESSED) && (note == nt))
    {
        start_level = adsr ();
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

inline float Key::adsr ()
{
    switch (status)
    {
    case KEY_PRESSED:
        if (time < envelope.attack)
        {
            return start_level + (1.0f - start_level) * time /envelope.attack;
        }

        if (time < envelope.attack + envelope.decay)
        {
            return 1.0f + (envelope.sustain - 1.0f) * (time - envelope.attack) / envelope.decay;
        }

        return envelope.sustain;

    case KEY_RELEASED:
        return start_level - start_level * time /envelope.release;

    default:
        return 0.0f;
    }
}

inline float Key::synthPartials()
{
    float value = 0.0f;
    for (int i=0; i<PARTIAL_NR; i++) {
        value += partials[i].amplitude *
                partials[i].attenuation *
                sin(2.0 * M_PI * (i+1) * position);
    }
    return value;
}

inline float Key::synth()
{
    const float p = fmod (position, 1.0);

    switch (waveform)
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
    return  adsr() *
            // synth () *
            synthPartials() *
            (static_cast<float> (velocity) / 127.0f) *
            fader.get();
}

inline void Key::proceed ()
{
    time += 1.0 / rate;
    position += freq / rate;
    if ((status == KEY_RELEASED) && (time >= envelope.release)) off();
}

inline bool Key::isOn () {
    return (status != KEY_OFF);
}

#endif /* KEY_HPP_ */
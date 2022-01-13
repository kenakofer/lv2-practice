/* include libs */
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cmath>
#include "lv2.h"
#include <lv2/atom/atom.h>
#include <lv2/urid/urid.h>
#include <lv2/midi/midi.h>
#include <lv2/core/lv2_util.h>
#include <lv2/atom/util.h>

// For debug
// #include <iostream>
// #include <string>
// #include <cstdio>

enum ControlPorts
{
    CONTROL_ATTACK = 0,
    CONTROL_DECAY = 1,
    CONTROL_SUSTAIN = 2,
    CONTROL_RELEASE = 3,
    CONTROL_LEVEL = 4,
    CONTROL_NR = 5
};

enum PortGroups
{
    PORT_MIDI_IN = 0,
    PORT_AUDIO_OUT = 1,
    PORT_CONTROL = 2,
    PORT_NR = 3
};

struct Urids
{
    LV2_URID midi_MidiEvent;
};

enum KeyStatus
{
    KEY_OFF,
    KEY_PRESSED,
    KEY_RELEASED
};

struct Envelope
{
    double attack;
    double decay;
    float sustain;
    double release;
};

class Key
{
private:
    KeyStatus status;
    uint8_t note;
    uint8_t velocity;
    Envelope envelope;
    double rate;
    double position;
    float start_level;
    double freq;
    double time;


public:
    Key (const double rt);
    void press (const uint8_t nt, const uint8_t vel, const Envelope env);
    void release (const uint8_t nt, const uint8_t vel);
    void off ();
    float get ();
    void proceed ();

private:
    float adsr ();

};

Key::Key(const double rt) :
    status (KEY_OFF),
    note (0),
    velocity (0),
    envelope {0.0, 0.0, 0.0f, 0.0},
    rate (rt),
    position (0.0),
    start_level (0.0f),
    freq (pow (2.0, (double (note) - 69.0) / 12.0) * 440.0),
    time (0.0)
{

}

void Key::press(const uint8_t nt, const uint8_t vel, const Envelope env)
{
    start_level = adsr();
    note = nt;
    velocity = vel;
    envelope = env;
    status = KEY_PRESSED;
    freq = pow (2.0, (double (note) - 69.0) / 12.0) * 440.0;
    time = 0.0;
}
void Key::release(const uint8_t nt, const uint8_t vel)
{
    if ((status == KEY_PRESSED) && (note == nt)) {
        start_level = adsr();
        time = 0.0;
        status = KEY_RELEASED;
    }
}
void Key::off()
{
    position = 0.0;
    status = KEY_OFF;
}

/* Return the current level of the envelope (0-1) */
inline float Key::adsr()
{
    switch (status)
    {
        case KEY_PRESSED:
            if (time < envelope.attack) {
                return start_level + (1.0 - start_level) * time / envelope.attack;
            }
            else if (time < envelope.attack + envelope.decay) {
                /* decay phase */
                return 1.0f + (time - envelope.attack) / envelope.decay * (envelope.sustain - 1.0f);
            } else {
                /* sustain phase */
                return envelope.sustain;
            }
        case KEY_RELEASED:
            if (time < envelope.release) {
                return start_level * (1.0f - time / envelope.release);
            }
            return 1.0f;
        default:
            return 0.0f;
    }
}

float Key::get()
{
    return  adsr() *
            sin (2.0 * M_PI * position) *
            (float (velocity) / 127.0f);
}
inline void Key::proceed()
{
    time += 1.0 / rate;
    position += freq / rate;
    if ((status == KEY_RELEASED) && (time >= envelope.release)) off();
}

/* class definiton */
class MySinSynth
{
private:
    const LV2_Atom_Sequence* midi_in_ptr;
    float* audio_out_ptr;
    const float* control_ptr[CONTROL_NR];
    Urids urids;
    double rate;
    double position;
    float actual_freq;
    float actual_level;
    LV2_URID_Map* map;
    Key key;

public:
    MySinSynth(const double sample_rate, const LV2_Feature *const *features);
    void connectPort(const uint32_t port, void* data_location);
    void activate();
    void run(const uint32_t sample_count);

private:
    void play (const uint32_t start, const uint32_t end);
};

MySinSynth::MySinSynth (const double sample_rate, const LV2_Feature *const *features) :
    midi_in_ptr (nullptr),
    audio_out_ptr (nullptr),
    control_ptr {nullptr},
    rate (sample_rate),
    position (0.0),
    actual_freq (0.0),
    map (nullptr),
    key (rate)
{
    const char* missing = lv2_features_query(
        features,
        LV2_URID__map,
        &map,
        true,
        NULL
    );

    if (missing) throw;

    urids.midi_MidiEvent = map->map(map->handle, LV2_MIDI__MidiEvent);
}

void MySinSynth::connectPort(const uint32_t port, void* data_location)
{
    switch (port)
    {
    case PORT_MIDI_IN:
        midi_in_ptr = (const LV2_Atom_Sequence*) data_location;
        break;
    case PORT_AUDIO_OUT:
        audio_out_ptr = (float*) data_location;
        break;
    default:
        if (port < PORT_CONTROL + CONTROL_NR)
        {
            control_ptr[port - PORT_CONTROL] = (const float*) data_location;
        } else throw;
        break;
    }
}

void MySinSynth::activate()
{
    position = 0.0;
    actual_freq = 440.0;
    actual_level = 0.1;
}


void MySinSynth::play (const uint32_t start, const uint32_t end)
{
    for (uint32_t i = start; i < end; ++i)
    {
        audio_out_ptr[i] = key.get() * *control_ptr[CONTROL_LEVEL];
        key.proceed();
    }
}

void MySinSynth::run(const uint32_t sample_count)
{
    if (!(audio_out_ptr && midi_in_ptr)) return;

    for (int i=0; i<CONTROL_NR; ++i) {
        if (!control_ptr[i]) return;
    }

    /* analyze incoming midi data */
    uint32_t last_frame = 0;
    LV2_ATOM_SEQUENCE_FOREACH (midi_in_ptr, ev)
    {
        /* play frames until event */
        const uint32_t frame = ev->time.frames;
        play (last_frame, frame);
        last_frame = frame;

        if (ev->body.type == urids.midi_MidiEvent) {
            const uint8_t* const msg = (const uint8_t*) (ev + 1);
            const uint8_t typ = lv2_midi_message_type (msg);

            switch (typ) {
                case LV2_MIDI_MSG_NOTE_ON:
                    key.press(
                        msg[1], /* note */
                        msg[2], /* velocity */
                        {
                            *control_ptr[CONTROL_ATTACK],
                            *control_ptr[CONTROL_DECAY],
                            *control_ptr[CONTROL_SUSTAIN],
                            *control_ptr[CONTROL_RELEASE]
                        }
                    );
                    break;
                case LV2_MIDI_MSG_NOTE_OFF:
                    key.release (msg[1], msg[2]);
                    break;
                case LV2_MIDI_MSG_CONTROLLER:
                    if (msg[1] == LV2_MIDI_CTL_ALL_NOTES_OFF) key.off();
                    else if (msg[1] == LV2_MIDI_CTL_ALL_SOUNDS_OFF) key.off();
                    break;
            }
        }
    }

    /* play remaining frames */
    play(last_frame, sample_count);
}




static LV2_Handle instantiate (const struct LV2_Descriptor *descriptor, double sample_rate, const char *bundle_path, const LV2_Feature *const *features)
{
    MySinSynth* m = new MySinSynth(sample_rate, features);
    return m;
}


static void connect_port (LV2_Handle instance, uint32_t port, void *data_location)
{
    MySinSynth* m = (MySinSynth*) instance;
    if (m) m->connectPort(port, data_location);
}
static void activate (LV2_Handle instance)
{
    MySinSynth* m = (MySinSynth*) instance;
    if (m) m->activate();
}
static void run (LV2_Handle instance, uint32_t sample_count)
{
    MySinSynth* m = (MySinSynth*) instance;
    if (m) m->run(sample_count);
}
static void deactivate (LV2_Handle instance)
{
    /* also not needed */
}
static void cleanup (LV2_Handle instance)
{
    MySinSynth* m = (MySinSynth*) instance;
    if (m) delete m;
}
static const void* extension_data (const char *uri)
{
    return NULL;
}

/* descriptor */
static LV2_Descriptor const descriptor =
{
    "https://github.com/kenakofer/lv2-practice/mySinSynth",
    instantiate,
    connect_port,
    activate, /* or NULL */
    run,
    deactivate, /* or NULL */
    cleanup,
    extension_data /* or NULL */
};

/* interface */

extern "C" LV2_SYMBOL_EXPORT const LV2_Descriptor * lv2_descriptor (uint32_t index)
{
    if (index==0) return &descriptor;
    else return NULL;

}

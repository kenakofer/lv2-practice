/* include libs */
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstdlib>
#include <cmath>
#include <stdexcept>
#include <iostream>
#include <new>
#include <array>
#include <random>
#include <ctime>
#include <utility>

#include "lv2.h"
#include <lv2/atom/atom.h>
#include <lv2/urid/urid.h>
#include <lv2/midi/midi.h>
#include <lv2/core/lv2_util.h>
#include <lv2/atom/util.h>

#include "BMap.hpp"
#include "Limit.hpp"
#include "LinearFader.hpp"
#include "Key.hpp"
#include "LowPassBasic.hpp"

enum ControlPorts
{
    CONTROL_WAVEFORM = 0,
    CONTROL_ATTACK   = 1,
    CONTROL_DECAY    = 2,
    CONTROL_SUSTAIN  = 3,
    CONTROL_RELEASE  = 4,
    CONTROL_LEVEL    = 5,
    CONTROL_NR       = 6
};

constexpr std::array<std::pair<float, float>, CONTROL_NR> controlLimit =
{{
    {0.0f, 4.0f},
    {0.001f, 4.0f},
    {0.001f, 4.0f},
    {0.0f, 1.0f},
    {0.001f, 4.0f},
    {0.0f, 1.0f},
}};

enum PortGroups
{
    PORT_MIDI_IN     = 0,
    PORT_AUDIO_OUT   = 1,
    PORT_CONTROL     = 2,
    PORT_NR          = 3
};

struct Urids
{
    LV2_URID midi_MidiEvent;
};

/* class definiton */
class MySimplePolySynth
{
private:
    const LV2_Atom_Sequence* midi_in_ptr;
    float* audio_out_ptr;
    std::array<const float*, CONTROL_NR> control_ptr;
    std::array<float, CONTROL_NR> control;
    LinearFader<float> controlLevel;
    Urids urids;
    double rate;
    double position;
    float actual_freq;
    float actual_level;
    LV2_URID_Map* map;
    BUtilities::BMap<uint8_t, Key, 128> key;
    LowPassBasic low_pass;


public:
    MySimplePolySynth(const double sample_rate, const LV2_Feature *const *features);
    void connectPort(const uint32_t port, void* data_location);
    void activate();
    void run(const uint32_t sample_count);

private:
    void play (const uint32_t start, const uint32_t end);
};

MySimplePolySynth::MySimplePolySynth (const double sample_rate, const LV2_Feature *const *features) :
    midi_in_ptr (nullptr),
    audio_out_ptr (nullptr),
    control_ptr {nullptr},
    rate (sample_rate),
    controlLevel (0.0f),
    position (0.0),
    actual_freq (0.0),
    map (nullptr),
    key (),
    low_pass ()
{
    control_ptr.fill(nullptr);
    control.fill(0.0f);

    const char* missing = lv2_features_query(
        features,
        LV2_URID__map,
        &map,
        true,
        NULL
    );

    if (missing) throw std::invalid_argument ("Feature map not provided by the host. Can't instantiate mySimplePolySynth");

    urids.midi_MidiEvent = map->map(map->handle, LV2_MIDI__MidiEvent);
}

void MySimplePolySynth::connectPort(const uint32_t port, void* data_location)
{
    switch (port)
    {
    case PORT_MIDI_IN:
        midi_in_ptr = static_cast<const LV2_Atom_Sequence*>(data_location);
        break;
    case PORT_AUDIO_OUT:
        audio_out_ptr = static_cast<float*>(data_location);
        break;
    default:
        if (port < PORT_CONTROL + CONTROL_NR)
        {
            control_ptr[port - PORT_CONTROL] = static_cast<const float*>(data_location);
        }
        break;
    }
}

void MySimplePolySynth::activate()
{
    position = 0.0;
    actual_freq = 440.0;
    actual_level = 0.1;
}


void MySimplePolySynth::play (const uint32_t start, const uint32_t end)
{
    for (uint32_t i = start; i < end; ++i)
    {
        audio_out_ptr[i] = 0.0f;
        for (BUtilities::BMap<uint8_t, Key, 128>::iterator it = key.begin(); it < key.end(); /* empty */) {
            BUtilities::BMap<uint8_t, Key, 128>::reference k = *it;

            if (k.second.isOn()) {
                audio_out_ptr[i] += k.second.get() * controlLevel.get();
                // audio_out_ptr[i] = low_pass.transform(audio_out_ptr[i]);
                k.second.proceed();
                ++it;
            } else {
                it = key.erase(it);
            }
        }
        controlLevel.proceed();
    }
}

void MySimplePolySynth::run(const uint32_t sample_count)
{
    /* check if all ports connected */
    if (!(audio_out_ptr && midi_in_ptr)) return;

    for (int i=0; i<CONTROL_NR; ++i) {
        if (!control_ptr[i]) return;
    }

    /* copy and validate control port values */
    for (int i=0; i<CONTROL_NR; ++i) {
        if (*control_ptr[i] != control[i]) {
            control[i] = limit<float> (*control_ptr[i], controlLimit[i].first, controlLimit[i].second);
            if (i == CONTROL_LEVEL) controlLevel.set (control[i], 0.01 * rate);
        }
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
            const uint8_t* const msg = reinterpret_cast<const uint8_t*>(ev + 1);
            const uint8_t typ = lv2_midi_message_type (msg);

            switch (typ) {
                case LV2_MIDI_MSG_NOTE_ON:
                    key[msg[1] & 0x7f].press(
                        static_cast<Waveform> (control[CONTROL_WAVEFORM]),
                        msg[1], /* note */
                        msg[2], /* velocity */
                        {
                            control[CONTROL_ATTACK],
                            control[CONTROL_DECAY],
                            control[CONTROL_SUSTAIN],
                            control[CONTROL_RELEASE]
                        }
                    );
                    break;
                case LV2_MIDI_MSG_NOTE_OFF:
                    key[msg[1] & 0x7f].release (msg[1], msg[2]);
                    break;
                case LV2_MIDI_MSG_CONTROLLER:
                    if (msg[1] == LV2_MIDI_CTL_ALL_NOTES_OFF) {
                        for (BUtilities::BMap<uint8_t, Key, 128>::reference k : key) {
                            k.second.release();
                        }
                    }
                    else if (msg[1] == LV2_MIDI_CTL_ALL_SOUNDS_OFF) {
                        for (BUtilities::BMap<uint8_t, Key, 128>::reference k : key) {
                            k.second.mute();
                        }
                    }
                    break;
            }
        }
    }

    /* play remaining frames */
    play(last_frame, sample_count);
}




static LV2_Handle instantiate (const struct LV2_Descriptor *descriptor, double sample_rate, const char *bundle_path, const LV2_Feature *const *features)
{
    MySimplePolySynth* m = nullptr;
    try {
        m = new MySimplePolySynth(sample_rate, features);
    } catch (const std::invalid_argument& ia) {
        std::cerr << ia.what() << std::endl;
        return nullptr;
    } catch (const std::bad_alloc& ba) {
        std::cerr << "Failed to allocate memory. Can't instantiate mySimplePolySynth" << std::endl;
        return nullptr;
    }
    return m;
}


static void connect_port (LV2_Handle instance, uint32_t port, void *data_location)
{
    MySimplePolySynth* m = static_cast<MySimplePolySynth*>(instance);
    if (m) m->connectPort(port, data_location);
}
static void activate (LV2_Handle instance)
{
    MySimplePolySynth* m = static_cast<MySimplePolySynth*>(instance);
    if (m) m->activate();
}
static void run (LV2_Handle instance, uint32_t sample_count)
{
    MySimplePolySynth* m = static_cast<MySimplePolySynth*>(instance);
    if (m) m->run(sample_count);
}
static void deactivate (LV2_Handle instance)
{
    /* also not needed */
}
static void cleanup (LV2_Handle instance)
{
    MySimplePolySynth* m = static_cast<MySimplePolySynth*>(instance);
    if (m) delete m;
}
static const void* extension_data (const char *uri)
{
    return NULL;
}

/* descriptor */
static LV2_Descriptor const descriptor =
{
    "https://github.com/kenakofer/lv2-practice/mySimplePolySynth",
    instantiate,
    connect_port,
    activate, /* or NULL */
    run,
    deactivate, /* or NULL */
    cleanup,
    extension_data /* or NULL */
};

/* interface */

LV2_SYMBOL_EXPORT const LV2_Descriptor * lv2_descriptor (uint32_t index)
{
    if (index==0) return &descriptor;
    else return NULL;

}

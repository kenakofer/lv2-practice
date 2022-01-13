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

enum ControlPorts
{
    CONTROL_ATTACK = 0,
    CONTROL_DECAY = 1,
    CONTROL_SUSTAIN = 2,
    CONTROL_RELEASE = 3,
    CONTROL_LEVEL = 4,
    CONTROL_NR = 5
}
enum PortGroups
{
    PORT_MIDI_IN = 0
    PORT_AUDIO_OUT = 1
    PORT_CONTROL = 2,
    PORT_NR = 3
}

struct Urids
{
    LV2_URID midi_MidiEvent;
};

/* class definiton */
class MySinSynth
{
private:
    const LV2_Atom_Sequence* midi_in_ptr;
    float* audio_out_ptr;
    const float* control_ptr[CONTROL_NR]
    Urids urids;
    double rate;
    double position;
    float actual_freq;
    float actual_level;
    LV2_URID_Map* map;

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
    actual_freq (0.0)
    map (nullptr);
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


void play (const uint32_t start, const uint32_t end)
{
    for (uint32_t i = start, i < end; ++i)
    {
        //TODO
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
            const uint8_t* const msg = (const uint8_t*) (ev + 1)
            const uint8_t typ = lv2_midi_message_type (msg);

            switch (type) {
                case LV2_MIDI_MSG_NOTE_ON:
                    break;
                case LV2_MIDI_MSG_NOTE_OFF:
                    break;
                case LV2_MIDI_MSG_CONTROLLER:
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

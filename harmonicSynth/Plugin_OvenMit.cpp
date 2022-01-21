#include "AudioPluginUtil.h"

namespace OvenMit
{
    enum Param
    {
        P_ENV_MODE_1 = 0,
        P_ATTACK     = 1,
        P_DECAY      = 2,
        P_SUSTAIN    = 3,
        P_RELEASE    = 4,

        P_WAVEFORM   = 5,
        P_LEVEL      = 6,
        P_PITCH      = 7,
        P_CUTOFF     = 8,
        P_RES_HEIGHT = 9,
        P_RES_WIDTH  = 10,

        P_ENV_MODE_2 = 11,
        P_ENV_AMT_2  = 12,
        P_ATTACK_2   = 13,
        P_DECAY_2    = 14,
        P_SUSTAIN_2  = 15,
        P_RELEASE_2  = 16,

        P_WAVEFORM_2_MODE = 17,
        P_WAVEFORM_2 = 18,
        P_LEVEL_2    = 19,
        P_PITCH_2    = 20,
        P_NUM         = 21
    };

    struct EffectData
    {
        struct Data
        {
            float p[P_NUM];
            float s;
            float c;
        };
        union
        {
            Data data;
            unsigned char pad[(sizeof(Data) + 15) & ~15]; // This entire structure must be a multiple of 16 bytes (and and instance 16 byte aligned) for PS3 SPU DMA requirements
        };
    };

    int InternalRegisterEffectDefinition(UnityAudioEffectDefinition& definition)
    {
        int numparams = P_NUM;
        definition.paramdefs = new UnityAudioParameterDefinition[numparams];
        RegisterParameter(definition, "EnvMode1", "", 0.0f, 3.0f, 0.0f, 1.0f, 1.0f, P_ENV_MODE_1, "Envelope 1 mode");
        RegisterParameter(definition, "Attack1", "s", 0.001f, 4.0f, 0.001f, 1.0f, 1.0f, P_ATTACK, "Attack time for envelope 1");
        RegisterParameter(definition, "Decay1", "s", 0.001f, 4.0f, 0.001f, 1.0f, 1.0f, P_DECAY, "Decay time for envelope 1");
        RegisterParameter(definition, "Sustain1", "", 0.000f, 1.0f, 0.5f, 1.0f, 1.0f, P_SUSTAIN, "Sustain level for envelope 1");
        RegisterParameter(definition, "Release1", "s", 0.001f, 4.0f, 0.001f, 1.0f, 1.0f, P_RELEASE, "Release time for envelope 1");

        RegisterParameter(definition, "Waveform1", "", 0.000f, 4.0f, 0.000f, 1.0f, 1.0f, P_WAVEFORM, "Waveform type for oscillator 1");
        RegisterParameter(definition, "Level1", "", 0.000f, 1.0f, 0.100f, 1.0f, 1.0f, P_LEVEL, "Fader level for oscillator 1");
        RegisterParameter(definition, "Pitch1", "", -24.0f, 24.0f, 0.0f, 1.0f, 1.0f, P_PITCH, "Pitch offset in semitones of oscillator 1");
        RegisterParameter(definition, "Cutoff1", "", 0.0f, 60.0f, 30.0f, 1.0f, 1.0f, P_CUTOFF, "The low-pass cutoff frequency, in partials, of oscillator 1");
        RegisterParameter(definition, "Resonance1", "", 0.0f, 8.0f, 3.0f, 1.0f, 1.0f, P_RES_HEIGHT, "Height of resonance around cutoff frequency");
        RegisterParameter(definition, "ResonanceWidth1", "", 1.0f, 30.0f, 3.0f, 1.0f, 1.0f, P_RES_WIDTH, "Width of resonance around cutoff frequency, in partials");

        RegisterParameter(definition, "EnvMode2", "", 0.0f, 3.0f, 0.0f, 1.0f, 1.0f, P_ENV_MODE_2, "Envelope 2 mode");
        RegisterParameter(definition, "EnvAmount2", "", 0.0f, 10.0f, 3.0f, 1.0f, 1.0f, P_ENV_AMT_2, "Amount envelope 2 should affect its target");
        RegisterParameter(definition, "Attack2", "s", 0.001f, 4.0f, 0.001f, 1.0f, 1.0f, P_ATTACK_2, "Attack time for envelope 2");
        RegisterParameter(definition, "Decay2", "s", 0.001f, 4.0f, 0.001f, 1.0f, 1.0f, P_DECAY_2, "Decay time for envelope 2");
        RegisterParameter(definition, "Sustain2", "", 0.000f, 1.0f, 0.5f, 1.0f, 1.0f, P_SUSTAIN_2, "Sustain level for envelope 2");
        RegisterParameter(definition, "Release2", "s", 0.001f, 4.0f, 0.001f, 1.0f, 1.0f, P_RELEASE_2, "Release time for envelope 2");

        RegisterParameter(definition, "Oscillator2Mode", "", 0.000f, 3.0f, 0.000f, 1.0f, 1.0f, P_WAVEFORM_2_MODE, "Target/mode of Oscillator 2");
        RegisterParameter(definition, "Waveform2", "", 0.000f, 4.0f, 0.000f, 1.0f, 1.0f, P_WAVEFORM_2, "Waveform type for oscillator 2");
        RegisterParameter(definition, "Level2", "", 0.000f, 1.0f, 0.100f, 1.0f, 1.0f, P_LEVEL_2, "Fader level for oscillator 2");
        RegisterParameter(definition, "Pitch2", "", -24.0f, 24.0f, -12.0f, 1.0f, 1.0f, P_PITCH_2, "Pitch offset in semitones of oscillator 2");

        return numparams;
    }
}
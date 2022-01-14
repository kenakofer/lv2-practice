#ifndef WAVEFORM_HPP_
#define WAVEFORM_HPP_

#include <array>

enum Waveform
{
    WAVEFORM_SINE       = 0,
    WAVEFORM_TRIANGLE   = 1,
    WAVEFORM_SQUARE     = 2,
    WAVEFORM_SAW        = 3,
    WAVEFORM_NR         = 4
};

const int PARTIAL_NR = 10;


constexpr std::array<std::array<float, PARTIAL_NR>, WAVEFORM_NR> PARTIAL_AMPLITUDES =
{{
    {1.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f},
    {1.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f}, //TODO
    {1.00f, 0.00f, 1.0f/3, 0.00f, 1.0f/5, 0.00f, 1.0f/7, 0.00f, 1.0f/9, 0.00f},
    {1.00f, -1.0f/2, -1.0f/3, -1.0f/4, -1.0f/5, -1.0f/6, -1.0f/7, -1.0f/8, -1.0f/9, -1.0f/10}
}};

#endif /* WAVEFORM_HPP_ */
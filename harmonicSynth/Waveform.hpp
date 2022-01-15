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

const int PARTIAL_NR = 30;


constexpr std::array<std::array<float, PARTIAL_NR>, WAVEFORM_NR> PARTIAL_AMPLITUDES =
{{
    {1.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f},
    {1.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f}, //TODO
    {1.00f, 0.00f, 1.0f/3, 0.00f, 1.0f/5, 0.00f, 1.0f/7, 0.00f, 1.0f/9, 0.00f, 1.0f/11, 0.00f, 1.0f/13, 0.00f, 1.0f/15, 0.00f, 1.0f/17, 0.00f, 1.0f/19, 0.00f, 1.0f/21, 0.00f, 1.0f/23, 0.00f, 1.0f/25, 0.00f, 1.0f/27, 0.00f, 1.0f/29, 0.00f},
    {1.00f, -1.0f/2, 1.0f/3, -1.0f/4, 1.0f/5, -1.0f/6, 1.0f/7, -1.0f/8, 1.0f/9, -1.0f/10, 1.0f/11, -1.0f/12, 1.0f/13, -1.0f/14, 1.0f/15, -1.0f/16, 1.0f/17, -1.0f/18, 1.0f/19, -1.0f/20 , 1.0f/21, -1.0f/22, 1.0f/23, -1.0f/24, 1.0f/25, -1.0f/26, 1.0f/27, -1.0f/28, 1.0f/29, -1.0f/30}
}};

#endif /* WAVEFORM_HPP_ */

#ifndef FILTER_HPP_
#define FILTER_HPP_

class Filter
{
    private:
        float peak_height;
        float peak_freq;
        float cutoff_freq;
        float flat_freq;
        float slope;

    public:
        Filter();
        float attenuationForFreq(float freq);
        void setValues(float cf, float pf, float ph);

    private:
        void _recalculate_values();
};

inline Filter::Filter () :
    peak_height (1.2f),
    peak_freq (5000.0f),
    cutoff_freq (6000.0f)
{
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
inline void Filter::setValues(float cf, float pf, float ph) {
    cutoff_freq = cf;
    peak_freq = pf;
    peak_height = ph;
    _recalculate_values();
}

inline void Filter::_recalculate_values() {
    slope = peak_height / (peak_freq - cutoff_freq);
    flat_freq = peak_freq + (peak_height - 1.0f) / slope;
}

#endif
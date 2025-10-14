#pragma once
#include <vector>
#include "Biquad.h"

namespace ezsqueeze::dsp {

class SidechainFilter {
public:
    void prepare(double newSampleRate, int numChannels);
    void reset();

    void setHighPassHz(float freqHz);
    void setLowPassHz(float freqHz);
    float getHighPassHz() const { return hpfHz; }
    float getLowPassHz() const { return lpfHz; }

    void process(float** audio, int numChannels, int numSamples);

private:
    void updateCoefficients();

    double sampleRate { 48000.0 };
    int channels { 0 };

    float hpfHz { 0.0f };   // 0 disables
    float lpfHz { 20000.0f }; // >= Nyquist disables

    std::vector<Biquad> hpf;
    std::vector<Biquad> lpf;
};

} // namespace ezsqueeze::dsp

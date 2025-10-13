#pragma once

#include <cstddef>
#include "Biquad.h"

namespace ezsqueeze::dsp {

class SidechainFilter {
public:
    void prepare(const double newSampleRate) noexcept {
        sampleRate = newSampleRate;
        hpf.prepare(sampleRate);
        lpf.prepare(sampleRate);
        update();
        reset();
    }

    void reset() noexcept {
        hpf.reset();
        lpf.reset();
    }

    void setHPFFrequencyHz(const float frequencyHz) noexcept {
        hpfFrequencyHz = frequencyHz;
        update();
    }

    void setLPFFrequencyHz(const float frequencyHz) noexcept {
        lpfFrequencyHz = frequencyHz;
        update();
    }

    float processSample(const float input) noexcept {
        float y = input;
        if (hpfEnabled) {
            y = hpf.processSample(y);
        }
        if (lpfEnabled) {
            y = lpf.processSample(y);
        }
        return y;
    }

    void setHPFEnabled(const bool enabled) noexcept { hpfEnabled = enabled; }
    void setLPFEnabled(const bool enabled) noexcept { lpfEnabled = enabled; }

private:
    void update() noexcept {
        if (hpfEnabled) hpf.makeHighPass(hpfFrequencyHz);
        if (lpfEnabled) lpf.makeLowPass(lpfFrequencyHz);
    }

    double sampleRate { 44100.0 };
    bool hpfEnabled { true };
    bool lpfEnabled { true };
    float hpfFrequencyHz { 80.0f };
    float lpfFrequencyHz { 8000.0f };
    Biquad hpf {};
    Biquad lpf {};
};

} // namespace ezsqueeze::dsp

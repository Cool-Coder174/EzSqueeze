#include "SidechainFilter.h"
#include <algorithm>

namespace ezsqueeze::dsp {

void SidechainFilter::prepare(double newSampleRate, int numChannels) {
    sampleRate = newSampleRate;
    channels = std::max(1, numChannels);
    hpf.assign(channels, Biquad{});
    lpf.assign(channels, Biquad{});
    updateCoefficients();
    reset();
}

void SidechainFilter::reset() {
    for (int ch = 0; ch < channels; ++ch) {
        hpf[ch].reset();
        lpf[ch].reset();
    }
}

void SidechainFilter::setHighPassHz(float freqHz) {
    hpfHz = std::max(0.0f, freqHz);
    updateCoefficients();
}

void SidechainFilter::setLowPassHz(float freqHz) {
    lpfHz = std::max(0.0f, freqHz);
    updateCoefficients();
}

void SidechainFilter::process(float** audio, int numChannelsIn, int numSamples) {
    const int chs = std::min(channels, std::max(1, numChannelsIn));

    for (int ch = 0; ch < chs; ++ch) {
        float* d = audio[ch];
        for (int n = 0; n < numSamples; ++n) {
            float x = d[n];
            if (hpfHz > 0.0f) x = hpf[ch].processSample(x);
            if (lpfHz > 0.0f && lpfHz < static_cast<float>(0.5 * sampleRate)) x = lpf[ch].processSample(x);
            d[n] = x;
        }
    }
}

void SidechainFilter::updateCoefficients() {
    const float nyquist = static_cast<float>(0.5 * sampleRate);

    if (hpfHz > 0.0f) {
        const auto c = makeHighPass(static_cast<float>(sampleRate), std::min(hpfHz, nyquist * 0.999f));
        for (auto& f : hpf) f.setCoefficients(c);
    }
    if (lpfHz > 0.0f && lpfHz < nyquist) {
        const auto c = makeLowPass(static_cast<float>(sampleRate), std::min(lpfHz, nyquist * 0.999f));
        for (auto& f : lpf) f.setCoefficients(c);
    }
}

} // namespace ezsqueeze::dsp

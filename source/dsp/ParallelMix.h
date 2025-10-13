#pragma once
#include <algorithm>

namespace ezsqueeze::dsp {

inline void parallelBlend(float** dryInOut, float** wetIn, int numChannels, int numSamples, float mix01) {
    mix01 = std::clamp(mix01, 0.0f, 1.0f);
    const float dryGain = 1.0f - mix01;
    const float wetGain = mix01;

    for (int ch = 0; ch < numChannels; ++ch) {
        float* dry = dryInOut[ch];
        const float* wet = wetIn[ch];
        for (int n = 0; n < numSamples; ++n) {
            dry[n] = dry[n] * dryGain + wet[n] * wetGain;
        }
    }
}

} // namespace ezsqueeze::dsp

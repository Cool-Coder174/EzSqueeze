#pragma once
#include <algorithm>
#include <cmath>

namespace ezsqueeze::dsp {

constexpr float kEpsilon = 1.0e-12f;

inline float clamp(float value, float lowerBound, float upperBound) {
    return std::min(upperBound, std::max(lowerBound, value));
}

inline float timeToCoeffMs(float timeMs, double sampleRate) {
    if (timeMs <= 0.0f || sampleRate <= 0.0) {
        return 1.0f;
    }
    const double tauSamples = (timeMs * 0.001) * sampleRate;
    return static_cast<float>(1.0 - std::exp(-1.0 / tauSamples));
}

inline float linearToDb(float linear) {
    return 20.0f * std::log10(std::max(linear, kEpsilon));
}

inline float dbToLinear(float db) {
    return std::pow(10.0f, db / 20.0f);
}

} // namespace ezsqueeze::dsp

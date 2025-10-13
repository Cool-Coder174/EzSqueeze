#pragma once

#include <algorithm>
#include <cmath>
#include <limits>
#include <type_traits>

namespace ezsqueeze::dsp {

constexpr float kEpsilon = 1.0e-12f;
constexpr float kMinDecibels = -120.0f;

template <typename T>
inline T clamp(const T value, const T minValue, const T maxValue) noexcept {
    return std::min(maxValue, std::max(value, minValue));
}

inline float decibelsToLinear(const float decibels) noexcept {
    return std::pow(10.0f, decibels / 20.0f);
}

inline float linearToDecibels(const float linear) noexcept {
    const float safe = std::max(linear, kEpsilon);
    return 20.0f * std::log10(safe);
}

inline float timeMsToCoeff(const float timeMilliseconds, const double sampleRate) noexcept {
    if (timeMilliseconds <= 0.0f) {
        return 1.0f; // immediate
    }
    const double timeInSamples = static_cast<double>(timeMilliseconds) * 0.001 * sampleRate;
    const double alpha = 1.0 - std::exp(-1.0 / timeInSamples);
    return static_cast<float>(alpha);
}

inline float timeMsToDecayCoefficient(const float timeMilliseconds, const double sampleRate) noexcept {
    if (timeMilliseconds <= 0.0f) {
        return 0.0f; // instant drop
    }
    const double timeInSamples = static_cast<double>(timeMilliseconds) * 0.001 * sampleRate;
    // Exponential decay factor used for peak hold fallback
    const double coeff = std::exp(-1.0 / timeInSamples);
    return static_cast<float>(coeff);
}

} // namespace ezsqueeze::dsp

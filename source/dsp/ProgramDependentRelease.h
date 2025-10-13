#pragma once

#include <cmath>
#include "Utilities.h"

namespace ezsqueeze::dsp {

class ProgramDependentRelease {
public:
    void prepare(const double newSampleRate) noexcept {
        sampleRate = newSampleRate;
        reset();
    }

    void reset() noexcept {
        previousInput = 0.0f;
        currentReleaseMs = baseReleaseMs;
    }

    void setBaseReleaseMs(const float ms) noexcept {
        baseReleaseMs = std::max(ms, 1.0f);
    }

    void setBoundsMs(const float minMs, const float maxMs) noexcept {
        minReleaseMs = std::max(1.0f, minMs);
        maxReleaseMs = std::max(minReleaseMs, maxMs);
    }

    float updateSuggestedReleaseMs(const float inputLevelLinear, const float grDecibels) noexcept {
        const float transient = std::fabs(inputLevelLinear - previousInput);
        previousInput = inputLevelLinear;

        // Heuristic: larger transients and larger GR -> faster release
        const bool largeTransient = transient > 0.05f; // tune
        const bool strongGR = std::fabs(grDecibels) > 3.0f;

        const float target = (largeTransient && strongGR) ? fastReleaseMs : slowReleaseMs;

        // Smooth move towards target
        const float coeff = 0.02f; // smoothing factor per call (assumes audio-rate calls)
        currentReleaseMs += coeff * (target - currentReleaseMs);
        currentReleaseMs = clamp(currentReleaseMs, minReleaseMs, maxReleaseMs);
        return currentReleaseMs;
    }

private:
    double sampleRate { 44100.0 };
    float previousInput { 0.0f };

    float minReleaseMs { 30.0f };
    float maxReleaseMs { 1000.0f };

    float baseReleaseMs { 200.0f };
    float fastReleaseMs { 60.0f };
    float slowReleaseMs { 300.0f };

    float currentReleaseMs { 200.0f };
};

} // namespace ezsqueeze::dsp

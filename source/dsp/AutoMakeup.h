#pragma once

#include <cmath>
#include "Utilities.h"

namespace ezsqueeze::dsp {

class AutoMakeupGain {
public:
    void setStaticEstimate(const float thresholdDecibels, const float ratio) noexcept {
        const float assumedOvershoot = 10.0f; // heuristic
        const float expectedGR = assumedOvershoot * (1.0f - 1.0f / std::max(ratio, 1.0f));
        staticMakeupDecibels = expectedGR * 0.7f;
    }

    void reset() noexcept { runningAverageGR = 0.0f; }

    void updateAdaptive(const float instantaneousGRDecibels, const float smoothingCoeff = 0.01f) noexcept {
        // instantaneousGRDecibels is negative or zero; average magnitude
        const float grMagnitude = std::fabs(instantaneousGRDecibels);
        runningAverageGR = (1.0f - smoothingCoeff) * runningAverageGR + smoothingCoeff * grMagnitude;
    }

    float getAdaptiveMakeupDecibels() const noexcept {
        return runningAverageGR * 0.8f;
    }

    float getStaticMakeupDecibels() const noexcept { return staticMakeupDecibels; }

private:
    float staticMakeupDecibels { 0.0f };
    float runningAverageGR { 0.0f };
};

} // namespace ezsqueeze::dsp

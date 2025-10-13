#pragma once
#include "MathUtils.h"

namespace ezsqueeze::dsp {

class AutoMakeup {
public:
    void setSmoothingMs(float ms, double sampleRate) {
        smoothingCoeff = timeToCoeffMs(ms, sampleRate);
    }

    // Simple heuristic based on threshold/ratio
    static float estimateMakeupDb(float thresholdDb, float ratio) {
        const float assumedOvershoot = 10.0f; // dB
        const float expectedGr = assumedOvershoot * (1.0f - 1.0f / std::max(1.0f, ratio));
        return expectedGr * 0.7f; // conservative
    }

    // Adaptive version using running average of actual GR (positive magnitude)
    float processAdaptive(float currentGrDb) {
        const float magnitude = std::max(0.0f, -currentGrDb);
        runningAverageGr = runningAverageGr + smoothingCoeff * (magnitude - runningAverageGr);
        return runningAverageGr * 0.8f; // slightly conservative
    }

    void reset() { runningAverageGr = 0.0f; }

private:
    float runningAverageGr { 0.0f };
    float smoothingCoeff { 0.05f };
};

} // namespace ezsqueeze::dsp

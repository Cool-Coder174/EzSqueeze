#pragma once
#include <algorithm>
#include "MathUtils.h"

namespace ezsqueeze::dsp {

class ProgramDependentRelease {
public:
    void prepare(double newSampleRate) { sampleRate = newSampleRate; }

    void setBaseReleaseMs(float ms) { baseReleaseMs = std::max(1.0f, ms); }
    void setFastReleaseMs(float ms) { fastReleaseMs = std::max(1.0f, ms); }
    void setSlowReleaseMs(float ms) { slowReleaseMs = std::max(1.0f, ms); }

    void setTransientThresholdDb(float db) { transientThresholdDb = db; }

    // Returns effective release coefficient to use in an envelope follower
    float computeReleaseCoeff(float inputLevelDb, float gainReductionDb) const {
        const float gr = std::max(0.0f, -gainReductionDb);
        const float transientMeasure = std::max(0.0f, inputLevelDb - lastInputDb);

        // Heuristic: if transient spike and significant GR, use fast release.
        float targetMs = baseReleaseMs;
        if (transientMeasure > std::max(0.0f, transientThresholdDb) && gr > 3.0f) {
            targetMs = fastReleaseMs;
        } else {
            // Interpolate between base and slow as GR increases
            const float t = std::clamp(gr / 12.0f, 0.0f, 1.0f);
            targetMs = baseReleaseMs * (1.0f - t) + slowReleaseMs * t;
        }
        return timeToCoeffMs(targetMs, sampleRate);
    }

    void updateLastInputDb(float db) { lastInputDb = db; }

private:
    double sampleRate { 48000.0 };
    float baseReleaseMs { 150.0f };
    float fastReleaseMs { 50.0f };
    float slowReleaseMs { 300.0f };
    float transientThresholdDb { 6.0f };

    float lastInputDb { -120.0f };
};

} // namespace ezsqueeze::dsp

#pragma once

#include <cmath>
#include "Utilities.h"
#include "EnvelopeFollower.h"

namespace ezsqueeze::dsp {

enum class DetectorMode {
    Peak,
    RMS
};

class DetectorEngine {
public:
    DetectorEngine() = default;

    void prepare(const double newSampleRate) noexcept {
        sampleRate = newSampleRate;
        setRmsWindowMilliseconds(rmsWindowMilliseconds);
        setPeakHoldMilliseconds(peakHoldMilliseconds);
        envelope.prepare(sampleRate);
        reset();
    }

    void reset() noexcept {
        previousRmsSquared = 0.0f;
        previousPeak = 0.0f;
        latestLevelLinear = 0.0f;
        envelope.reset();
    }

    void setMode(const DetectorMode newMode) noexcept { mode = newMode; }

    void setRmsWindowMilliseconds(const float windowMs) noexcept {
        rmsWindowMilliseconds = std::max(windowMs, 0.1f);
        rmsAlpha = timeMsToCoeff(rmsWindowMilliseconds, sampleRate);
    }

    void setPeakHoldMilliseconds(const float holdMs) noexcept {
        peakHoldMilliseconds = std::max(holdMs, 1.0f);
        peakDecayCoeff = timeMsToDecayCoefficient(peakHoldMilliseconds, sampleRate);
    }

    void setEnvelopeTimesMilliseconds(const float attackMs, const float releaseMs) noexcept {
        envelope.setAttackMilliseconds(attackMs);
        envelope.setReleaseMilliseconds(releaseMs);
    }

    float processSampleLinear(const float inputSample) noexcept {
        const float x = std::fabs(inputSample);
        float levelLinear = 0.0f;
        if (mode == DetectorMode::Peak) {
            // Instant rise, exponential fall
            const float decayed = previousPeak * peakDecayCoeff;
            levelLinear = std::max(x, decayed);
            previousPeak = levelLinear;
        } else {
            // Exponential moving average of squared signal -> sqrt
            previousRmsSquared = (1.0f - rmsAlpha) * previousRmsSquared + rmsAlpha * (x * x);
            levelLinear = std::sqrt(std::max(previousRmsSquared, 0.0f));
        }
        latestLevelLinear = envelope.processSample(levelLinear);
        return latestLevelLinear;
    }

    float processSampleDecibels(const float inputSample) noexcept {
        const float level = processSampleLinear(inputSample);
        return linearToDecibels(level);
    }

    [[nodiscard]] float getLatestLevelLinear() const noexcept { return latestLevelLinear; }
    [[nodiscard]] float getLatestLevelDecibels() const noexcept { return linearToDecibels(latestLevelLinear); }

private:
    double sampleRate { 44100.0 };
    DetectorMode mode { DetectorMode::RMS };

    // Peak detector
    float peakHoldMilliseconds { 200.0f };
    float peakDecayCoeff { 0.0f };
    float previousPeak { 0.0f };

    // RMS detector
    float rmsWindowMilliseconds { 5.0f };
    float rmsAlpha { 0.0f };
    float previousRmsSquared { 0.0f };

    // Envelope after detection
    EnvelopeFollower envelope {};

    float latestLevelLinear { 0.0f };
};

} // namespace ezsqueeze::dsp

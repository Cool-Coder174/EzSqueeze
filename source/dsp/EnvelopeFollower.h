#pragma once

#include <cmath>
#include "Utilities.h"

namespace ezsqueeze::dsp {

class EnvelopeFollower {
public:
    EnvelopeFollower() = default;

    void prepare(const double newSampleRate) noexcept {
        sampleRate = newSampleRate;
        updateCoefficients();
        reset();
    }

    void reset() noexcept {
        envelopeState = 0.0f;
    }

    void setAttackMilliseconds(const float attackMs) noexcept {
        attackMilliseconds = std::max(attackMs, 0.0f);
        updateCoefficients();
    }

    void setReleaseMilliseconds(const float releaseMs) noexcept {
        releaseMilliseconds = std::max(releaseMs, 0.0f);
        updateCoefficients();
    }

    float processSample(const float inputValue) noexcept {
        const float target = std::max(inputValue, 0.0f);
        if (target > envelopeState) {
            // Attack (rise)
            envelopeState += attackCoefficient * (target - envelopeState);
        } else {
            // Release (fall)
            envelopeState += releaseCoefficient * (target - envelopeState);
        }
        return envelopeState;
    }

    [[nodiscard]] float getState() const noexcept { return envelopeState; }

private:
    void updateCoefficients() noexcept {
        attackCoefficient = timeMsToCoeff(attackMilliseconds, sampleRate);
        releaseCoefficient = timeMsToCoeff(releaseMilliseconds, sampleRate);
    }

    double sampleRate { 44100.0 };
    float attackMilliseconds { 5.0f };
    float releaseMilliseconds { 100.0f };
    float attackCoefficient { 0.0f };
    float releaseCoefficient { 0.0f };
    float envelopeState { 0.0f };
};

} // namespace ezsqueeze::dsp

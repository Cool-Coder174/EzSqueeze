#pragma once

#include <cmath>
#include "Utilities.h"

namespace ezsqueeze::dsp {

enum class KneeType {
    Hard,
    Medium,
    Soft
};

class GainComputer {
public:
    static float computeGainReductionDecibels(const float inputDecibels,
                                              const float thresholdDecibels,
                                              const float ratio,
                                              const KneeType kneeType) noexcept {
        const float kneeWidth = (kneeType == KneeType::Hard ? 0.0f : (kneeType == KneeType::Medium ? 3.0f : 6.0f));
        return computeWithKnee(inputDecibels, thresholdDecibels, std::max(ratio, 1.0f), kneeWidth);
    }

    static float reductionDecibelsToLinearGain(const float reductionDecibels) noexcept {
        // reductionDecibels is negative or zero; convert to linear gain (<= 1.0)
        return decibelsToLinear(reductionDecibels);
    }

private:
    static float computeHardKnee(const float inputDecibels,
                                 const float thresholdDecibels,
                                 const float ratio) noexcept {
        if (inputDecibels <= thresholdDecibels) {
            return 0.0f; // no reduction
        }
        const float overshoot = inputDecibels - thresholdDecibels;
        const float reduction = overshoot * (1.0f - 1.0f / ratio);
        return -reduction; // negative dB
    }

    static float computeWithKnee(const float inputDecibels,
                                 const float thresholdDecibels,
                                 const float ratio,
                                 const float kneeWidthDecibels) noexcept {
        if (kneeWidthDecibels <= 0.0f) {
            return computeHardKnee(inputDecibels, thresholdDecibels, ratio);
        }
        const float halfKnee = 0.5f * kneeWidthDecibels;
        const float lower = thresholdDecibels - halfKnee;
        const float upper = thresholdDecibels + halfKnee;

        if (inputDecibels <= lower) {
            return 0.0f;
        }
        if (inputDecibels >= upper) {
            return computeHardKnee(inputDecibels, thresholdDecibels, ratio);
        }
        const float x = inputDecibels - lower; // 0..kneeWidth
        const float t = x / kneeWidthDecibels;  // 0..1
        const float hard = computeHardKnee(inputDecibels, thresholdDecibels, ratio);
        // Parabolic interpolation within knee
        const float kneeFactor = t * t;
        return hard * kneeFactor;
    }
};

} // namespace ezsqueeze::dsp

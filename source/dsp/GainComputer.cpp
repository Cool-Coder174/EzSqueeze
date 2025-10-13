#include "GainComputer.h"
#include <algorithm>

namespace ezsqueeze::dsp {

void GainComputer::setKneeType(KneeType type) {
    kneeType = type;
    switch (kneeType) {
        case KneeType::Hard:   kneeWidthDb = 0.0f; break;
        case KneeType::Medium: kneeWidthDb = 3.0f; break;
        case KneeType::Soft:   kneeWidthDb = 6.0f; break;
    }
}

float GainComputer::computeGainReductionDb(float inputDb) const {
    const float overshoot = inputDb - thresholdDb;

    if (kneeWidthDb <= 1.0e-6f) {
        // Hard knee
        if (overshoot <= 0.0f) return 0.0f;
        const float reduction = overshoot * (1.0f - 1.0f / ratio);
        return -reduction;
    }

    // Soft/Medium knee: transition region
    const float kneeLower = thresholdDb - kneeWidthDb * 0.5f;
    const float kneeUpper = thresholdDb + kneeWidthDb * 0.5f;

    if (inputDb <= kneeLower) return 0.0f;

    const float baseReduction = overshoot > 0.0f ? overshoot * (1.0f - 1.0f / ratio) : 0.0f;

    if (inputDb >= kneeUpper) return -baseReduction;

    // Within knee: parabolic interpolation (0..1)
    const float x = inputDb - kneeLower; // 0 .. kneeWidth
    const float t = x / kneeWidthDb;     // 0 .. 1
    const float kneeFactor = t * t;      // Smooth ramp-up
    return -baseReduction * kneeFactor;
}

} // namespace ezsqueeze::dsp

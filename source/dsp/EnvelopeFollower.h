#pragma once
#include "MathUtils.h"

namespace ezsqueeze::dsp {

class EnvelopeFollower {
public:
    void prepare(double newSampleRate);
    void reset(float initialValue = 0.0f);

    void setAttackMs(float timeMs);
    void setReleaseMs(float timeMs);

    float getAttackMs() const { return attackMs; }
    float getReleaseMs() const { return releaseMs; }

    // Processes absolute/positive input values (typ. detector output)
    float processSample(float input);

private:
    void updateCoefficients();

    double sampleRate { 48000.0 };
    float attackMs { 10.0f };
    float releaseMs { 100.0f };
    float attackCoeff { 0.0f };
    float releaseCoeff { 0.0f };
    float currentEnvelope { 0.0f };
};

} // namespace ezsqueeze::dsp

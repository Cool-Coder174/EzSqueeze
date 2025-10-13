#include "EnvelopeFollower.h"
#include <algorithm>

namespace ezsqueeze::dsp {

void EnvelopeFollower::prepare(double newSampleRate) {
    sampleRate = newSampleRate;
    updateCoefficients();
    reset(0.0f);
}

void EnvelopeFollower::reset(float initialValue) {
    currentEnvelope = initialValue;
}

void EnvelopeFollower::setAttackMs(float timeMs) {
    attackMs = std::max(0.0f, timeMs);
    updateCoefficients();
}

void EnvelopeFollower::setReleaseMs(float timeMs) {
    releaseMs = std::max(0.0f, timeMs);
    updateCoefficients();
}

float EnvelopeFollower::processSample(float input) {
    const float target = std::max(0.0f, input);
    const bool isRising = target > currentEnvelope;
    const float coeff = isRising ? attackCoeff : releaseCoeff;
    currentEnvelope = currentEnvelope + coeff * (target - currentEnvelope);
    return currentEnvelope;
}

void EnvelopeFollower::updateCoefficients() {
    attackCoeff = timeToCoeffMs(attackMs, sampleRate);
    releaseCoeff = timeToCoeffMs(releaseMs, sampleRate);
}

} // namespace ezsqueeze::dsp

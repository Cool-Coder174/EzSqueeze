#include "EnvelopeFollower.h"
#include <algorithm>
#include <cmath>

namespace EzSqueeze::DSP {

EnvelopeFollower::EnvelopeFollower(float sampleRate, float attackMs, float releaseMs)
    : sampleRate_(sampleRate)
    , attackMs_(0.0f)
    , releaseMs_(0.0f)
    , attackCoeff_(0.0f)
    , releaseCoeff_(0.0f)
    , envelopeLevel_(0.0f)
    , isAttacking_(false)
{
    prepare(sampleRate, attackMs, releaseMs);
}

void EnvelopeFollower::prepare(float sampleRate, float attackMs, float releaseMs)
{
    sampleRate_ = sampleRate;
    attackMs_ = clampTime(attackMs);
    releaseMs_ = clampTime(releaseMs);
    
    // Calculate filter coefficients
    attackCoeff_ = timeToCoeff(attackMs_);
    releaseCoeff_ = timeToCoeff(releaseMs_);
    
    // Reset state
    reset();
}

float EnvelopeFollower::processSample(float input)
{
    // Determine if we're in attack or release phase
    if (input > envelopeLevel_)
    {
        // Attack phase
        isAttacking_ = true;
        envelopeLevel_ = envelopeLevel_ + attackCoeff_ * (input - envelopeLevel_);
    }
    else
    {
        // Release phase
        isAttacking_ = false;
        envelopeLevel_ = envelopeLevel_ + releaseCoeff_ * (input - envelopeLevel_);
    }
    
    return envelopeLevel_;
}

void EnvelopeFollower::processBlock(const float* input, float* output, int numSamples)
{
    for (int i = 0; i < numSamples; ++i)
    {
        output[i] = processSample(input[i]);
    }
}

void EnvelopeFollower::reset()
{
    envelopeLevel_ = 0.0f;
    isAttacking_ = false;
}

void EnvelopeFollower::setAttackTime(float attackMs)
{
    attackMs_ = clampTime(attackMs);
    attackCoeff_ = timeToCoeff(attackMs_);
}

void EnvelopeFollower::setReleaseTime(float releaseMs)
{
    releaseMs_ = clampTime(releaseMs);
    releaseCoeff_ = timeToCoeff(releaseMs_);
}

float EnvelopeFollower::timeToCoeff(float timeMs) const
{
    if (timeMs <= 0.0f)
    {
        return 1.0f;  // Instant response
    }
    
    // Convert time constant to coefficient
    // For 63% response time τ (in samples): α = 1 - e^(-1/τ)
    float tau = timeMs * 0.001f * sampleRate_;  // Convert ms to samples
    return 1.0f - std::exp(-1.0f / tau);
}

float EnvelopeFollower::clampTime(float timeMs) const
{
    return std::max(0.0f, timeMs);
}

} // namespace EzSqueeze::DSP
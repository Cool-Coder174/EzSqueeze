/**
 * @file EnvelopeFollower.cpp
 * @brief Implementation of attack/release envelope
 */

#include "EnvelopeFollower.h"

namespace EzSqueeze {
namespace DSP {

void EnvelopeFollower::prepare(double sampleRate)
{
    m_sampleRate = sampleRate;
    reset();
}

void EnvelopeFollower::setAttack(float attackMs)
{
    attackMs = std::clamp(attackMs, 0.1f, 100.0f);
    m_attackCoeff = timeToCoeff(attackMs);
}

void EnvelopeFollower::setRelease(float releaseMs)
{
    releaseMs = std::clamp(releaseMs, 10.0f, 1000.0f);
    m_releaseCoeff = timeToCoeff(releaseMs);
}

float EnvelopeFollower::processSample(float input)
{
    // Use attack coefficient when envelope is rising (more compression needed)
    // Use release coefficient when envelope is falling (less compression needed)
    
    // Note: For gain reduction, more negative = more compression
    // So "rising" means becoming more negative
    const bool isRising = input < m_envelope;
    
    const float coeff = isRising ? m_attackCoeff : m_releaseCoeff;
    
    // One-pole low-pass filter
    m_envelope = m_envelope + coeff * (input - m_envelope);
    
    return m_envelope;
}

void EnvelopeFollower::reset()
{
    m_envelope = 0.0f;
}

float EnvelopeFollower::timeToCoeff(float timeMs) const
{
    // Convert time constant to coefficient for 63% response
    // Formula: alpha = 1 - exp(-1 / (time_ms * sample_rate / 1000))
    const float timeSamples = (timeMs / 1000.0f) * static_cast<float>(m_sampleRate);
    return 1.0f - std::exp(-1.0f / timeSamples);
}

} // namespace DSP
} // namespace EzSqueeze

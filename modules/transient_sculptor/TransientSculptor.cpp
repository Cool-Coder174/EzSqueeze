/**
 * @file TransientSculptor.cpp
 * @brief Implementation of transient shaping
 */

#include "TransientSculptor.h"

namespace EzSqueeze {
namespace Modules {

void TransientSculptor::prepare(double sampleRate)
{
    m_sampleRate = sampleRate;
    updateLimiterCoeffs();
    reset();
}

void TransientSculptor::setSnap(float snap)
{
    m_snap = std::clamp(snap, 0.0f, 1.0f);
}

void TransientSculptor::setBody(float body)
{
    m_body = std::clamp(body, 0.0f, 1.0f);
}

void TransientSculptor::setDeSnap(float deSnap)
{
    m_deSnap = std::clamp(deSnap, 0.0f, 1.0f);
    updateLimiterCoeffs();
}

float TransientSculptor::getModifiedAttack(float baseAttackMs) const
{
    // Snap: reduces attack time (faster = more transient)
    // Map snap 0→1 to multiplier 1.0→0.5 (faster)
    const float attackBias = 1.0f - m_snap * 0.5f;
    return baseAttackMs * attackBias;
}

float TransientSculptor::getModifiedRelease(float baseReleaseMs) const
{
    // Body: increases release time (slower = more sustain)
    // Map body 0→1 to multiplier 1.0→2.0 (slower)
    const float releaseBias = 1.0f + m_body;
    return baseReleaseMs * releaseBias;
}

float TransientSculptor::applyDeSnap(float input)
{
    if (m_deSnap < 0.001f)
    {
        return input;  // Bypass if de-snap is off
    }

    const float absInput = std::abs(input);

    // Fast envelope follower for transient detection
    if (absInput > m_transientPeak)
    {
        m_transientPeak = absInput;  // Instant attack
    }
    else
    {
        m_transientPeak += m_fastReleaseCoeff * (absInput - m_transientPeak);
    }

    // Calculate gain reduction for transients
    const float threshold = 0.5f;  // Threshold for what counts as a transient
    if (m_transientPeak > threshold)
    {
        const float overshoot = m_transientPeak - threshold;
        const float reduction = 1.0f / (1.0f + overshoot * m_deSnap * 2.0f);
        return input * reduction;
    }

    return input;
}

void TransientSculptor::reset()
{
    m_transientPeak = 0.0f;
}

void TransientSculptor::updateLimiterCoeffs()
{
    // Very fast attack (0.1ms)
    m_fastAttackCoeff = timeToCoeff(0.1f);

    // Fast release (10ms)
    m_fastReleaseCoeff = timeToCoeff(10.0f);
}

float TransientSculptor::timeToCoeff(float timeMs) const
{
    const float timeSamples = (timeMs / 1000.0f) * static_cast<float>(m_sampleRate);
    return 1.0f - std::exp(-1.0f / timeSamples);
}

} // namespace Modules
} // namespace EzSqueeze


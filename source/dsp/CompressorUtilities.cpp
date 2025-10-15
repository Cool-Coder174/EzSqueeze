/**
 * @file CompressorUtilities.cpp
 * @brief Implementation of compressor utility classes
 */

#include "CompressorUtilities.h"

namespace EzSqueeze {
namespace DSP {

// ============================================================================
// ParallelMix Implementation
// ============================================================================

void ParallelMix::setMix(float mixPercent)
{
    const float mix = std::clamp(mixPercent, 0.0f, 100.0f) / 100.0f;
    
    // Equal power crossfade
    const float angle = mix * 1.57079632679f;  // 0 to pi/2
    m_wetGain = std::sin(angle);
    m_dryGain = std::cos(angle);
}

float ParallelMix::process(float dry, float wet) const
{
    return dry * m_dryGain + wet * m_wetGain;
}

// ============================================================================
// AutoMakeupGain Implementation
// ============================================================================

void AutoMakeupGain::prepare(double sampleRate)
{
    m_sampleRate = sampleRate;
    m_adaptiveGain = 0.0f;
}

void AutoMakeupGain::setEnabled(bool enabled)
{
    m_enabled = enabled;
}

float AutoMakeupGain::estimateMakeupGain(float threshold, float ratio) const
{
    if (!m_enabled)
    {
        return 0.0f;
    }
    
    // Estimate average overshoot for typical program material
    // Assume signal peaks ~10dB above threshold on average
    const float assumedOvershoot = 10.0f;
    
    // Calculate expected GR
    const float expectedGR = assumedOvershoot * (1.0f - 1.0f / ratio);
    
    // Apply 70% of expected GR to avoid over-compensation
    return expectedGR * 0.7f;
}

void AutoMakeupGain::updateAdaptive(float currentGR)
{
    if (!m_enabled)
    {
        return;
    }
    
    // Smooth adaptation to actual GR
    // Convert GR (negative) to positive makeup
    const float targetGain = -currentGR * 0.8f;  // 80% compensation
    
    m_adaptiveGain = m_adaptiveGain + m_smoothingCoeff * (targetGain - m_adaptiveGain);
}

float AutoMakeupGain::getMakeupGain() const
{
    return m_enabled ? m_adaptiveGain : 0.0f;
}

// ============================================================================
// ProgramDependentRelease Implementation
// ============================================================================

void ProgramDependentRelease::prepare(double sampleRate)
{
    m_sampleRate = sampleRate;
    m_prevLevel = -120.0f;
}

void ProgramDependentRelease::setBaseRelease(float releaseMs)
{
    m_baseRelease = std::clamp(releaseMs, 10.0f, 1000.0f);
}

void ProgramDependentRelease::setFastRelease(float releaseMs)
{
    m_fastRelease = std::clamp(releaseMs, 10.0f, 200.0f);
}

float ProgramDependentRelease::computeReleaseTime(float inputLevel, float grAmount)
{
    // Detect transients by measuring level change
    const float levelChange = std::abs(inputLevel - m_prevLevel);
    m_prevLevel = inputLevel;
    
    // If we detect a transient AND have significant GR, use fast release
    const bool isTransient = (levelChange > m_transientThreshold);
    const bool significantGR = (grAmount > 3.0f);  // More than 3dB GR
    
    if (isTransient && significantGR)
    {
        return m_fastRelease;  // Fast release to prevent pumping
    }
    else
    {
        return m_baseRelease;  // Normal release for smooth material
    }
}

} // namespace DSP
} // namespace EzSqueeze


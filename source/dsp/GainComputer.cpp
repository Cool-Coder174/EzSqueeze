/**
 * @file GainComputer.cpp
 * @brief Implementation of gain reduction computation
 */

#include "GainComputer.h"

namespace EzSqueeze {
namespace DSP {

void GainComputer::setThreshold(float thresholdDb)
{
    m_threshold = std::clamp(thresholdDb, -60.0f, 0.0f);
}

void GainComputer::setRatio(float ratio)
{
    m_ratio = std::clamp(ratio, 1.0f, 32.0f);
}

void GainComputer::setKneeMode(KneeMode mode)
{
    m_kneeMode = mode;
    updateKneeWidth();
}

float GainComputer::computeGainReduction(float inputLevelDb) const
{
    // Below knee start: no compression
    const float kneeStart = m_threshold - m_kneeWidth / 2.0f;
    
    if (inputLevelDb <= kneeStart)
    {
        return 0.0f;  // No gain reduction
    }
    
    // Above knee end: full ratio compression
    const float kneeEnd = m_threshold + m_kneeWidth / 2.0f;
    
    if (inputLevelDb >= kneeEnd)
    {
        const float overshoot = inputLevelDb - m_threshold;
        return -computeHardKnee(overshoot);  // Negative for reduction
    }
    
    // Within knee region: smooth transition
    if (m_kneeMode == KneeMode::Hard)
    {
        // Hard knee is effectively instant transition
        const float overshoot = inputLevelDb - m_threshold;
        return -computeHardKnee(overshoot);
    }
    else
    {
        // Soft/medium knee: parabolic interpolation
        return -computeSoftKnee(inputLevelDb);
    }
}

void GainComputer::updateKneeWidth()
{
    switch (m_kneeMode)
    {
        case KneeMode::Hard:
            m_kneeWidth = 0.1f;  // Very narrow transition
            break;
        case KneeMode::Medium:
            m_kneeWidth = 3.0f;  // 3 dB transition
            break;
        case KneeMode::Soft:
            m_kneeWidth = 6.0f;  // 6 dB transition
            break;
    }
}

float GainComputer::computeSoftKnee(float inputLevelDb) const
{
    // Position within knee (0 to 1)
    const float kneeStart = m_threshold - m_kneeWidth / 2.0f;
    const float x = (inputLevelDb - kneeStart) / m_kneeWidth;
    
    // Parabolic curve for smooth knee
    // At x=0: no compression, at x=1: full ratio
    const float kneeFactor = x * x;  // Quadratic curve
    
    // Compute as if at threshold, then scale by knee factor
    const float overshoot = inputLevelDb - m_threshold;
    const float fullReduction = computeHardKnee(std::max(0.0f, overshoot + m_kneeWidth / 2.0f));
    
    return fullReduction * kneeFactor;
}

} // namespace DSP
} // namespace EzSqueeze

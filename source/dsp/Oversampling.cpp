/**
 * @file Oversampling.cpp
 * @brief Implementation of oversampling engine
 */

#include "Oversampling.h"

namespace EzSqueeze {
namespace DSP {

// ============================================================================
// SimpleOversampler Implementation
// ============================================================================

void SimpleOversampler::prepare(double sampleRate, OversamplingFactor maxFactor)
{
    m_baseSampleRate = sampleRate;
    reset();
}

void SimpleOversampler::setFactor(OversamplingFactor factor)
{
    m_factor = factor;
}

void SimpleOversampler::reset()
{
    m_previousInput = 0.0f;
}

// ============================================================================
// OversamplingController Implementation
// ============================================================================

void OversamplingController::setFactor(OversamplingFactor factor)
{
    m_requestedFactor = factor;
}

void OversamplingController::setEcoMode(bool enabled)
{
    m_ecoMode = enabled;
}

OversamplingFactor OversamplingController::getEffectiveFactor() const
{
    // Eco Mode forces oversampling off
    if (m_ecoMode)
    {
        return OversamplingFactor::Off;
    }

    return m_requestedFactor;
}

float OversamplingController::getCPUCostEstimate() const
{
    const OversamplingFactor effective = getEffectiveFactor();

    switch (effective)
    {
        case OversamplingFactor::Off:
            return 1.0f;
        case OversamplingFactor::X2:
            return 2.5f;  // 2× processing + filter overhead
        case OversamplingFactor::X4:
            return 5.0f;  // 4× processing + filter overhead
        case OversamplingFactor::X8:
            return 10.0f; // 8× processing + filter overhead
        default:
            return 1.0f;
    }
}

} // namespace DSP
} // namespace EzSqueeze


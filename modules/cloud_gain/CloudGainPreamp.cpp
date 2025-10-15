/**
 * @file CloudGainPreamp.cpp
 * @brief Implementation of Cloud-Gain preamp
 */

#include "CloudGainPreamp.h"

namespace EzSqueeze {
namespace Modules {

void CloudGainPreamp::prepare(double sampleRate)
{
    m_sampleRate = sampleRate;
    updateLFBumpFilter();
    reset();
}

void CloudGainPreamp::setGain(float gainDb)
{
    gainDb = std::clamp(gainDb, 0.0f, 30.0f);
    m_gainLinear = std::pow(10.0f, gainDb / 20.0f);
}

void CloudGainPreamp::setImpedanceMode(ImpedanceMode mode)
{
    m_mode = mode;
}

void CloudGainPreamp::setCharacterAmount(float amount)
{
    m_characterAmount = std::clamp(amount, 0.0f, 1.0f);
}

float CloudGainPreamp::processSample(float input)
{
    // Apply clean gain
    float output = input * m_gainLinear;

    // Apply character if enabled
    if (m_characterAmount > 0.001f)
    {
        output = applyCharacter(output);
    }

    return output;
}

void CloudGainPreamp::reset()
{
    m_lfBumpZ1 = 0.0f;
}

float CloudGainPreamp::applyCharacter(float sample)
{
    switch (m_mode)
    {
        case ImpedanceMode::Silicon:
            // Minimal character - just very soft saturation
            return sample + m_characterAmount * 0.01f * softSaturate(sample * 2.0f);

        case ImpedanceMode::Tube:
            // Even harmonics (warm, smooth)
            return evenHarmonics(sample);

        case ImpedanceMode::Transformer:
            // Even + odd harmonics with LF bump
            return transformerColor(sample);

        default:
            return sample;
    }
}

float CloudGainPreamp::transformerColor(float x)
{
    // Apply LF bump (simple one-pole shelf)
    const float lfBumped = x + m_lfBumpCoeff * (x - m_lfBumpZ1);
    m_lfBumpZ1 = x;

    // Add even harmonics
    const float withHarmonics = lfBumped + m_characterAmount * 0.03f * (lfBumped * lfBumped);

    // Add subtle odd harmonics (saturation)
    const float withOdd = withHarmonics + m_characterAmount * 0.02f * (withHarmonics * withHarmonics * withHarmonics);

    return withOdd;
}

void CloudGainPreamp::updateLFBumpFilter()
{
    // Simple shelf filter coefficient for ~100Hz bump
    // Higher sample rates need different coefficients
    const float freq = 100.0f / static_cast<float>(m_sampleRate);
    m_lfBumpCoeff = freq * 0.5f;  // Subtle boost
}

} // namespace Modules
} // namespace EzSqueeze


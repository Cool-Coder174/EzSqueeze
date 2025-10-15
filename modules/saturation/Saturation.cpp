/**
 * @file Saturation.cpp
 * @brief Implementation of saturation processor
 */

#include "Saturation.h"

namespace EzSqueeze {
namespace Modules {

void Saturation::setCurve(SaturationCurve curve)
{
    m_curve = curve;
    updateGains();
}

void Saturation::setDrive(float drive)
{
    m_drive = std::clamp(drive, 0.0f, 1.0f);
    updateGains();
}

void Saturation::setTone(float tone)
{
    m_tone = std::clamp(tone, -1.0f, 1.0f);
}

float Saturation::processSample(float input)
{
    if (m_curve == SaturationCurve::Off || m_drive < 0.001f)
    {
        return input;
    }

    float output = input;

    // Apply selected saturation curve
    switch (m_curve)
    {
        case SaturationCurve::SoftClip:
            output = softClip(input);
            break;

        case SaturationCurve::EvenHarmonics:
            output = evenHarmonicsSat(input);
            break;

        case SaturationCurve::OddHarmonics:
            output = oddHarmonicsSat(input);
            break;

        case SaturationCurve::Tape:
            output = tapeSaturation(input);
            break;

        default:
            break;
    }

    // Apply tone control
    output = applyTone(output);

    // Apply makeup gain compensation
    output *= m_compensationGain;

    return output;
}

void Saturation::updateGains()
{
    // Map drive (0-1) to gain (1-10)
    m_driveGain = 1.0f + m_drive * 9.0f;

    // Compensate for perceived loudness increase
    m_compensationGain = 1.0f / (1.0f + m_drive * 0.3f);
}

float Saturation::tapeSaturation(float x) const
{
    const float driven = x * m_driveGain;

    // Asymmetric saturation (different for positive/negative)
    if (driven > 0.0f)
    {
        // Positive side: softer saturation
        return std::tanh(driven * 0.8f) * 1.25f;
    }
    else
    {
        // Negative side: harder saturation
        return std::tanh(driven * 1.2f) * 0.83f;
    }
}

} // namespace Modules
} // namespace EzSqueeze


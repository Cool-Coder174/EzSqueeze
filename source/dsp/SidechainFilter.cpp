/**
 * @file SidechainFilter.cpp
 * @brief Implementation of sidechain filtering
 */

#include "SidechainFilter.h"

namespace EzSqueeze {
namespace DSP {

constexpr float PI = 3.14159265358979323846f;

// ============================================================================
// BiquadFilter Implementation
// ============================================================================

void BiquadFilter::makeHighPass(double sampleRate, float frequency, float Q)
{
    calculateCoefficients(sampleRate, frequency, Q, true);
}

void BiquadFilter::makeLowPass(double sampleRate, float frequency, float Q)
{
    calculateCoefficients(sampleRate, frequency, Q, false);
}

float BiquadFilter::processSample(float input)
{
    // Direct Form I biquad
    // y[n] = b0*x[n] + b1*x[n-1] + b2*x[n-2] - a1*y[n-1] - a2*y[n-2]
    
    const float output = b0 * input + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;
    
    // Update state
    x2 = x1;
    x1 = input;
    y2 = y1;
    y1 = output;
    
    return output;
}

void BiquadFilter::reset()
{
    x1 = x2 = y1 = y2 = 0.0f;
}

void BiquadFilter::calculateCoefficients(double sampleRate, float frequency,
                                         float Q, bool isHighPass)
{
    // Robert Bristow-Johnson's Audio EQ Cookbook formulas
    const float w0 = 2.0f * PI * frequency / static_cast<float>(sampleRate);
    const float cosw0 = std::cos(w0);
    const float sinw0 = std::sin(w0);
    const float alpha = sinw0 / (2.0f * Q);
    
    if (isHighPass)
    {
        // HPF coefficients
        const float a0 = 1.0f + alpha;
        b0 = (1.0f + cosw0) / 2.0f / a0;
        b1 = -(1.0f + cosw0) / a0;
        b2 = (1.0f + cosw0) / 2.0f / a0;
        a1 = -2.0f * cosw0 / a0;
        a2 = (1.0f - alpha) / a0;
    }
    else
    {
        // LPF coefficients
        const float a0 = 1.0f + alpha;
        b0 = (1.0f - cosw0) / 2.0f / a0;
        b1 = (1.0f - cosw0) / a0;
        b2 = (1.0f - cosw0) / 2.0f / a0;
        a1 = -2.0f * cosw0 / a0;
        a2 = (1.0f - alpha) / a0;
    }
}

// ============================================================================
// SidechainFilter Implementation
// ============================================================================

void SidechainFilter::prepare(double sampleRate)
{
    m_sampleRate = sampleRate;
    
    // Initialize filters
    m_highPass.makeHighPass(sampleRate, m_hpfFreq);
    m_lowPass.makeLowPass(sampleRate, m_lpfFreq);
    
    reset();
}

void SidechainFilter::setHighPassFrequency(float frequency)
{
    m_hpfFreq = std::clamp(frequency, 20.0f, 400.0f);
    m_highPass.makeHighPass(m_sampleRate, m_hpfFreq);
}

void SidechainFilter::setLowPassFrequency(float frequency)
{
    m_lpfFreq = std::clamp(frequency, 4000.0f, 16000.0f);
    m_lowPass.makeLowPass(m_sampleRate, m_lpfFreq);
}

void SidechainFilter::setHighPassEnabled(bool enabled)
{
    m_hpfEnabled = enabled;
}

void SidechainFilter::setLowPassEnabled(bool enabled)
{
    m_lpfEnabled = enabled;
}

float SidechainFilter::processSample(float input)
{
    float output = input;
    
    // Apply HPF if enabled
    if (m_hpfEnabled)
    {
        output = m_highPass.processSample(output);
    }
    
    // Apply LPF if enabled
    if (m_lpfEnabled)
    {
        output = m_lowPass.processSample(output);
    }
    
    return output;
}

void SidechainFilter::reset()
{
    m_highPass.reset();
    m_lowPass.reset();
}

} // namespace DSP
} // namespace EzSqueeze

#include "SidechainFilter.h"
#include <algorithm>
#include <cmath>

namespace EzSqueeze::DSP {

SidechainFilter::SidechainFilter(float sampleRate, float hpfFreq, float lpfFreq)
    : sampleRate_(sampleRate)
    , hpfFreq_(0.0f)
    , lpfFreq_(0.0f)
    , hpfCoeffs_{}
    , lpfCoeffs_{}
    , hpfState_{}
    , lpfState_{}
{
    prepare(sampleRate, hpfFreq, lpfFreq);
}

void SidechainFilter::prepare(float sampleRate, float hpfFreq, float lpfFreq)
{
    sampleRate_ = sampleRate;
    hpfFreq_ = clampFrequency(hpfFreq);
    lpfFreq_ = clampFrequency(lpfFreq);
    
    // Calculate filter coefficients
    if (isHPFEnabled())
    {
        hpfCoeffs_ = calculateButterworthCoeffs(hpfFreq_, FilterType::HighPass);
    }
    
    if (isLPFEnabled())
    {
        lpfCoeffs_ = calculateButterworthCoeffs(lpfFreq_, FilterType::LowPass);
    }
    
    // Reset filter states
    reset();
}

float SidechainFilter::processSample(float input)
{
    float output = input;
    
    // Apply high-pass filter if enabled
    if (isHPFEnabled())
    {
        output = processBiquad(output, hpfCoeffs_, hpfState_);
    }
    
    // Apply low-pass filter if enabled
    if (isLPFEnabled())
    {
        output = processBiquad(output, lpfCoeffs_, lpfState_);
    }
    
    return output;
}

void SidechainFilter::processBlock(const float* input, float* output, int numSamples)
{
    for (int i = 0; i < numSamples; ++i)
    {
        output[i] = processSample(input[i]);
    }
}

void SidechainFilter::reset()
{
    resetBiquadState(hpfState_);
    resetBiquadState(lpfState_);
}

void SidechainFilter::setHPFFrequency(float freq)
{
    hpfFreq_ = clampFrequency(freq);
    if (isHPFEnabled())
    {
        hpfCoeffs_ = calculateButterworthCoeffs(hpfFreq_, FilterType::HighPass);
    }
    resetBiquadState(hpfState_);
}

void SidechainFilter::setLPFFrequency(float freq)
{
    lpfFreq_ = clampFrequency(freq);
    if (isLPFEnabled())
    {
        lpfCoeffs_ = calculateButterworthCoeffs(lpfFreq_, FilterType::LowPass);
    }
    resetBiquadState(lpfState_);
}

SidechainFilter::BiquadCoefficients SidechainFilter::calculateButterworthCoeffs(float freq, FilterType type) const
{
    BiquadCoefficients coeffs{};
    
    // Clamp frequency to valid range
    freq = std::max(1.0f, std::min(freq, sampleRate_ * 0.5f - 1.0f));
    
    // Calculate normalized frequency
    float w = 2.0f * M_PI * freq / sampleRate_;
    float cosw = std::cos(w);
    float sinw = std::sin(w);
    
    // Butterworth 2nd-order coefficients
    float alpha = sinw / (2.0f * std::sqrt(2.0f));  // Q = 1/sqrt(2) for Butterworth
    
    if (type == FilterType::LowPass)
    {
        // Low-pass filter coefficients
        float b0 = (1.0f - cosw) * 0.5f;
        float b1 = 1.0f - cosw;
        float b2 = (1.0f - cosw) * 0.5f;
        float a0 = 1.0f + alpha;
        float a1 = -2.0f * cosw;
        float a2 = 1.0f - alpha;
        
        // Normalize coefficients
        coeffs.b0 = b0 / a0;
        coeffs.b1 = b1 / a0;
        coeffs.b2 = b2 / a0;
        coeffs.a1 = a1 / a0;
        coeffs.a2 = a2 / a0;
    }
    else // HighPass
    {
        // High-pass filter coefficients
        float b0 = (1.0f + cosw) * 0.5f;
        float b1 = -(1.0f + cosw);
        float b2 = (1.0f + cosw) * 0.5f;
        float a0 = 1.0f + alpha;
        float a1 = -2.0f * cosw;
        float a2 = 1.0f - alpha;
        
        // Normalize coefficients
        coeffs.b0 = b0 / a0;
        coeffs.b1 = b1 / a0;
        coeffs.b2 = b2 / a0;
        coeffs.a1 = a1 / a0;
        coeffs.a2 = a2 / a0;
    }
    
    return coeffs;
}

float SidechainFilter::processBiquad(float input, const BiquadCoefficients& coeffs, BiquadState& state)
{
    // Biquad filter: y[n] = b0*x[n] + b1*x[n-1] + b2*x[n-2] - a1*y[n-1] - a2*y[n-2]
    float output = coeffs.b0 * input + 
                   coeffs.b1 * state.x1 + 
                   coeffs.b2 * state.x2 - 
                   coeffs.a1 * state.y1 - 
                   coeffs.a2 * state.y2;
    
    // Update state
    state.x2 = state.x1;
    state.x1 = input;
    state.y2 = state.y1;
    state.y1 = output;
    
    return output;
}

void SidechainFilter::resetBiquadState(BiquadState& state)
{
    state.x1 = state.x2 = 0.0f;
    state.y1 = state.y2 = 0.0f;
}

float SidechainFilter::clampFrequency(float freq) const
{
    if (freq <= 0.0f)
        return 0.0f;  // Disabled
    
    // Clamp to valid range (1 Hz to Nyquist - 1 Hz)
    return std::max(1.0f, std::min(freq, sampleRate_ * 0.5f - 1.0f));
}

} // namespace EzSqueeze::DSP
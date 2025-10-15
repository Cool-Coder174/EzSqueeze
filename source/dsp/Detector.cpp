/**
 * @file Detector.cpp
 * @brief Implementation of level detection engine
 */

#include "Detector.h"

namespace EzSqueeze {
namespace DSP {

void Detector::prepare(double sampleRate, float rmsWindowMs)
{
    m_sampleRate = sampleRate;
    m_rmsAlpha = calculateRmsAlpha(rmsWindowMs);
    reset();
}

void Detector::setMode(DetectorMode mode)
{
    m_mode = mode;
}

float Detector::processSample(float input)
{
    const float absInput = std::abs(input);
    float level = 0.0f;
    
    switch (m_mode)
    {
        case DetectorMode::Peak:
        {
            // Peak detector with decay
            if (absInput > m_peakLevel)
            {
                m_peakLevel = absInput;  // Instant attack
            }
            else
            {
                m_peakLevel *= m_peakDecay;  // Slow decay
            }
            level = m_peakLevel;
            break;
        }
        
        case DetectorMode::RMS:
        {
            // RMS detector using exponential averaging
            const float squared = input * input;
            m_rmsSquared = m_rmsSquared + m_rmsAlpha * (squared - m_rmsSquared);
            level = std::sqrt(std::max(m_rmsSquared, 0.0f));
            break;
        }
    }
    
    // Convert to dB scale
    return linearToDb(level);
}

void Detector::reset()
{
    m_peakLevel = 0.0f;
    m_rmsSquared = 0.0f;
}

float Detector::calculateRmsAlpha(float windowMs)
{
    // Convert window time to samples
    const float windowSamples = (windowMs * 0.001f) * static_cast<float>(m_sampleRate);
    
    // Calculate exponential averaging coefficient
    // For 63% response time: alpha = 1 - exp(-1/tau)
    return 1.0f - std::exp(-1.0f / windowSamples);
}

} // namespace DSP
} // namespace EzSqueeze

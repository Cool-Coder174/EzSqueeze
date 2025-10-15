/**
 * @file LookaheadBuffer.cpp
 * @brief Implementation of lookahead delay buffer
 */

#include "LookaheadBuffer.h"

namespace EzSqueeze {
namespace DSP {

void LookaheadBuffer::prepare(double sampleRate, float maxLookaheadMs)
{
    m_sampleRate = sampleRate;
    
    // Calculate max buffer size
    m_maxDelaySamples = static_cast<int>(
        std::ceil(maxLookaheadMs * 0.001 * sampleRate)
    );
    
    // Allocate buffer (this is the only allocation - happens in prepare())
    m_buffer.resize(m_maxDelaySamples + 1, 0.0f);
    
    reset();
}

void LookaheadBuffer::setDelay(float delayMs)
{
    m_delayMs = std::clamp(delayMs, 0.0f, 
                           static_cast<float>(m_maxDelaySamples) / static_cast<float>(m_sampleRate) * 1000.0f);
    
    // Convert ms to samples
    m_delaySamples = static_cast<int>(
        std::round(m_delayMs * 0.001 * m_sampleRate)
    );
}

float LookaheadBuffer::processSample(float input)
{
    if (m_delaySamples == 0 || m_buffer.empty())
    {
        return input;  // No delay, pass through
    }
    
    // Calculate read position (circular buffer)
    const int bufferSize = static_cast<int>(m_buffer.size());
    const int readPos = (m_writePos - m_delaySamples + bufferSize) % bufferSize;
    
    // Read delayed sample
    const float output = m_buffer[readPos];
    
    // Write new sample
    m_buffer[m_writePos] = input;
    
    // Advance write position
    m_writePos = (m_writePos + 1) % bufferSize;
    
    return output;
}

void LookaheadBuffer::reset()
{
    std::fill(m_buffer.begin(), m_buffer.end(), 0.0f);
    m_writePos = 0;
}

} // namespace DSP
} // namespace EzSqueeze

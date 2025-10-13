#include "LookaheadBuffer.h"
#include <algorithm>
#include <cmath>

namespace EzSqueeze::DSP {

LookaheadBuffer::LookaheadBuffer(float maxDelayMs, float sampleRate)
    : writePos_(0)
    , delaySamples_(0)
    , delayMs_(0.0f)
    , sampleRate_(sampleRate)
    , maxDelayMs_(maxDelayMs)
    , maxDelaySamples_(0)
{
    prepare(sampleRate, maxDelayMs);
}

void LookaheadBuffer::prepare(float sampleRate, float maxDelayMs)
{
    sampleRate_ = sampleRate;
    maxDelayMs_ = maxDelayMs;
    maxDelaySamples_ = msToSamples(maxDelayMs);
    
    // Pre-allocate buffer with maximum possible size
    buffer_.resize(maxDelaySamples_, 0.0f);
    writePos_ = 0;
    
    // Set default delay to 0
    setDelay(0.0f);
}

void LookaheadBuffer::setDelay(float delayMs)
{
    delayMs_ = clampDelay(delayMs);
    delaySamples_ = msToSamples(delayMs_);
}

float LookaheadBuffer::processSample(float input)
{
    if (buffer_.empty())
    {
        return input;  // No delay if buffer not prepared
    }
    
    // Calculate read position (delay samples before write position)
    int readPos = (writePos_ - delaySamples_ + maxDelaySamples_) % maxDelaySamples_;
    
    // Get delayed output
    float output = buffer_[readPos];
    
    // Store current input for future output
    buffer_[writePos_] = input;
    
    // Advance write position
    writePos_ = (writePos_ + 1) % maxDelaySamples_;
    
    return output;
}

void LookaheadBuffer::processBlock(const float* input, float* output, int numSamples)
{
    for (int i = 0; i < numSamples; ++i)
    {
        output[i] = processSample(input[i]);
    }
}

void LookaheadBuffer::reset()
{
    if (!buffer_.empty())
    {
        std::fill(buffer_.begin(), buffer_.end(), 0.0f);
    }
    writePos_ = 0;
}

int LookaheadBuffer::msToSamples(float delayMs) const
{
    return static_cast<int>(delayMs * 0.001f * sampleRate_);
}

float LookaheadBuffer::samplesToMs(int delaySamples) const
{
    return static_cast<float>(delaySamples) / sampleRate_ * 1000.0f;
}

float LookaheadBuffer::clampDelay(float delayMs) const
{
    return std::max(0.0f, std::min(delayMs, maxDelayMs_));
}

} // namespace EzSqueeze::DSP
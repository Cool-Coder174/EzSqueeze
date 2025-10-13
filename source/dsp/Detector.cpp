#include "Detector.h"
#include <algorithm>

namespace EzSqueeze::DSP {

DetectorEngine::DetectorEngine(float sampleRate, Mode mode, float rmsWindowMs)
    : mode_(mode)
    , sampleRate_(sampleRate)
    , rmsWindowSamples_(0)
    , peakLevel_(0.0f)
    , peakDecayCoeff_(PEAK_DECAY_RATE)
    , rmsLevel_(0.0f)
    , rmsSquared_(0.0f)
    , rmsAlpha_(0.0f)
{
    prepare(sampleRate, mode, rmsWindowMs);
}

void DetectorEngine::prepare(float sampleRate, Mode mode, float rmsWindowMs)
{
    sampleRate_ = sampleRate;
    mode_ = mode;
    
    // Calculate RMS window size in samples
    rmsWindowSamples_ = static_cast<int>(rmsWindowMs * 0.001f * sampleRate_);
    
    // Calculate RMS smoothing coefficient
    // For 63% response time: α = 1 - e^(-1/τ) where τ is window size in samples
    float tau = static_cast<float>(rmsWindowSamples_);
    rmsAlpha_ = 1.0f - std::exp(-1.0f / tau);
    
    // Reset state
    reset();
}

float DetectorEngine::processSample(float input)
{
    float detectedLevel;
    
    if (mode_ == Mode::Peak)
    {
        updatePeak(input);
        detectedLevel = peakLevel_;
    }
    else // RMS mode
    {
        updateRMS(input);
        detectedLevel = rmsLevel_;
    }
    
    return linearToDB(detectedLevel);
}

void DetectorEngine::processBlock(const float* input, float* output, int numSamples)
{
    for (int i = 0; i < numSamples; ++i)
    {
        output[i] = processSample(input[i]);
    }
}

void DetectorEngine::reset()
{
    peakLevel_ = 0.0f;
    rmsLevel_ = 0.0f;
    rmsSquared_ = 0.0f;
}

float DetectorEngine::linearToDB(float linearLevel) const
{
    // Clamp to minimum level to avoid log(0)
    linearLevel = std::max(linearLevel, MIN_LEVEL);
    return DB_CONVERSION * std::log10(linearLevel);
}

void DetectorEngine::updateRMS(float input)
{
    // Update RMS squared using exponential moving average
    float inputSquared = input * input;
    rmsSquared_ = (1.0f - rmsAlpha_) * rmsSquared_ + rmsAlpha_ * inputSquared;
    
    // Calculate RMS level
    rmsLevel_ = std::sqrt(rmsSquared_);
}

void DetectorEngine::updatePeak(float input)
{
    // Update peak with decay
    float absInput = std::abs(input);
    peakLevel_ = std::max(absInput, peakLevel_ * peakDecayCoeff_);
}

} // namespace EzSqueeze::DSP
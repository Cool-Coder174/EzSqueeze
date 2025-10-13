#include "AutoMakeupGain.h"
#include <algorithm>
#include <cmath>

namespace EzSqueeze::DSP {

AutoMakeupGain::AutoMakeupGain(Mode mode, float manualGain)
    : mode_(mode)
    , manualGain_(0.0f)
    , threshold_(0.0f)
    , ratio_(1.0f)
    , currentMakeupGain_(0.0f)
    , adaptiveGain_(0.0f)
    , runningAverageGR_(0.0f)
    , alpha_(ADAPTIVE_ALPHA)
{
    prepare(mode, manualGain, 0.0f, 1.0f);
}

void AutoMakeupGain::prepare(Mode mode, float manualGain, float threshold, float ratio)
{
    mode_ = mode;
    manualGain_ = clampGain(manualGain);
    threshold_ = threshold;
    ratio_ = std::max(1.0f, ratio);
    
    // Calculate initial makeup gain
    switch (mode_)
    {
        case Mode::Off:
            currentMakeupGain_ = manualGain_;
            break;
            
        case Mode::Static:
            currentMakeupGain_ = calculateStaticMakeup(threshold_, ratio_);
            break;
            
        case Mode::Adaptive:
            currentMakeupGain_ = manualGain_;
            adaptiveGain_ = 0.0f;
            runningAverageGR_ = 0.0f;
            break;
    }
}

float AutoMakeupGain::processSample(float input, float gainReduction)
{
    // Update adaptive gain if in adaptive mode
    if (mode_ == Mode::Adaptive)
    {
        updateAdaptiveGain(gainReduction);
        currentMakeupGain_ = manualGain_ + adaptiveGain_;
    }
    
    // Apply makeup gain
    float makeupLinear = std::pow(10.0f, currentMakeupGain_ / 20.0f);
    return input * makeupLinear;
}

void AutoMakeupGain::processBlock(const float* input, float* output, 
                                 const float* gainReduction, int numSamples)
{
    for (int i = 0; i < numSamples; ++i)
    {
        output[i] = processSample(input[i], gainReduction[i]);
    }
}

void AutoMakeupGain::updateCompressionParams(float threshold, float ratio)
{
    threshold_ = threshold;
    ratio_ = std::max(1.0f, ratio);
    
    // Recalculate static makeup gain if in static mode
    if (mode_ == Mode::Static)
    {
        currentMakeupGain_ = calculateStaticMakeup(threshold_, ratio_);
    }
}

void AutoMakeupGain::setManualGain(float gain)
{
    manualGain_ = clampGain(gain);
    
    // Update current makeup gain based on mode
    switch (mode_)
    {
        case Mode::Off:
            currentMakeupGain_ = manualGain_;
            break;
            
        case Mode::Static:
            currentMakeupGain_ = calculateStaticMakeup(threshold_, ratio_);
            break;
            
        case Mode::Adaptive:
            currentMakeupGain_ = manualGain_ + adaptiveGain_;
            break;
    }
}

void AutoMakeupGain::setMode(Mode mode)
{
    mode_ = mode;
    
    // Recalculate makeup gain for new mode
    switch (mode_)
    {
        case Mode::Off:
            currentMakeupGain_ = manualGain_;
            break;
            
        case Mode::Static:
            currentMakeupGain_ = calculateStaticMakeup(threshold_, ratio_);
            break;
            
        case Mode::Adaptive:
            currentMakeupGain_ = manualGain_ + adaptiveGain_;
            break;
    }
}

void AutoMakeupGain::reset()
{
    if (mode_ == Mode::Adaptive)
    {
        adaptiveGain_ = 0.0f;
        runningAverageGR_ = 0.0f;
        currentMakeupGain_ = manualGain_;
    }
}

float AutoMakeupGain::calculateStaticMakeup(float /*threshold*/, float ratio) const
{
    // Estimate average gain reduction based on typical material characteristics
    // Assume material typically overshoots threshold by ASSUMED_OVERSHOOT dB
    float expectedOvershoot = ASSUMED_OVERSHOOT;
    float expectedGR = expectedOvershoot * (1.0f - 1.0f / ratio);
    
    // Apply compensation factor to avoid over-compensation
    return expectedGR * COMPENSATION_FACTOR;
}

void AutoMakeupGain::updateAdaptiveGain(float gainReduction)
{
    // Convert gain reduction to positive value for averaging
    float grMagnitude = std::abs(gainReduction);
    
    // Update running average of gain reduction
    runningAverageGR_ = (1.0f - alpha_) * runningAverageGR_ + alpha_ * grMagnitude;
    
    // Calculate adaptive makeup gain
    adaptiveGain_ = runningAverageGR_ * COMPENSATION_FACTOR;
}

float AutoMakeupGain::clampGain(float gain) const
{
    // Clamp to reasonable range (-60 dB to +60 dB)
    return std::max(-60.0f, std::min(60.0f, gain));
}

} // namespace EzSqueeze::DSP
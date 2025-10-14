#include "GainComputer.h"

namespace EzSqueeze::DSP {

GainComputer::GainComputer(float threshold, float ratio, KneeType knee)
    : threshold_(threshold)
    , ratio_(ratio)
    , knee_(knee)
    , kneeWidth_(0.0f)
    , kneeStart_(0.0f)
    , kneeEnd_(0.0f)
{
    updateKneeParameters();
}

void GainComputer::setParameters(float threshold, float ratio, KneeType knee)
{
    threshold_ = threshold;
    ratio_ = std::max(1.0f, ratio);  // Ensure ratio >= 1.0
    knee_ = knee;
    updateKneeParameters();
}

float GainComputer::calculateGainReduction(float inputLevel) const
{
    // No compression below threshold
    if (inputLevel <= threshold_)
    {
        return 0.0f;
    }

    // Apply different knee calculations based on type
    switch (knee_)
    {
        case KneeType::Hard:
            return calculateHardKnee(inputLevel);
            
        case KneeType::Medium:
        case KneeType::Soft:
            return calculateSoftKnee(inputLevel);
            
        default:
            return calculateHardKnee(inputLevel);
    }
}

void GainComputer::processBlock(const float* inputLevels, float* gainReduction, int numSamples) const
{
    for (int i = 0; i < numSamples; ++i)
    {
        gainReduction[i] = calculateGainReduction(inputLevels[i]);
    }
}

float GainComputer::getKneeWidth() const
{
    return kneeWidth_;
}

bool GainComputer::isInKneeRegion(float inputLevel) const
{
    return inputLevel > kneeStart_ && inputLevel < kneeEnd_;
}

void GainComputer::updateKneeParameters()
{
    switch (knee_)
    {
        case KneeType::Hard:
            kneeWidth_ = 0.0f;
            kneeStart_ = threshold_;
            kneeEnd_ = threshold_;
            break;
            
        case KneeType::Medium:
            kneeWidth_ = 3.0f;
            kneeStart_ = threshold_ - kneeWidth_ * 0.5f;
            kneeEnd_ = threshold_ + kneeWidth_ * 0.5f;
            break;
            
        case KneeType::Soft:
            kneeWidth_ = 6.0f;
            kneeStart_ = threshold_ - kneeWidth_ * 0.5f;
            kneeEnd_ = threshold_ + kneeWidth_ * 0.5f;
            break;
    }
}

float GainComputer::calculateHardKnee(float inputLevel) const
{
    if (inputLevel <= threshold_)
    {
        return 0.0f;
    }
    
    float overshoot = inputLevel - threshold_;
    float reduction = overshoot * (1.0f - 1.0f / ratio_);
    return -reduction;  // Return negative value (gain reduction)
}

float GainComputer::calculateSoftKnee(float inputLevel) const
{
    if (inputLevel <= kneeStart_)
    {
        return 0.0f;  // Below knee region
    }
    else if (inputLevel >= kneeEnd_)
    {
        return calculateHardKnee(inputLevel);  // Above knee region
    }
    else
    {
        // Within knee region - use parabolic interpolation
        float x = inputLevel - kneeStart_;
        float normalizedX = x / kneeWidth_;
        
        // Parabolic curve: y = x^2 for smooth transition
        float kneeFactor = normalizedX * normalizedX;
        
        // Interpolate between no compression and full compression
        float hardKneeReduction = calculateHardKnee(inputLevel);
        return kneeFactor * hardKneeReduction;
    }
}

float GainComputer::lerp(float a, float b, float t) const
{
    t = clamp(t, 0.0f, 1.0f);
    return a + t * (b - a);
}

float GainComputer::clamp(float value, float min, float max) const
{
    return std::max(min, std::min(max, value));
}

} // namespace EzSqueeze::DSP
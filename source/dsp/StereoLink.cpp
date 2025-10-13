#include "StereoLink.h"
#include <algorithm>
#include <cmath>

namespace EzSqueeze::DSP {

StereoLink::StereoLink(float linkAmount, bool msMode)
    : linkAmount_(0.0f)
    , msMode_(false)
    , isEncoding_(false)
{
    prepare(linkAmount, msMode);
}

void StereoLink::prepare(float linkAmount, bool msMode)
{
    linkAmount_ = clampLinkAmount(linkAmount);
    msMode_ = msMode;
    isEncoding_ = false;  // Start in L/R mode
}

void StereoLink::processDetectionLevels(float leftLevel, float rightLevel, 
                                       float& leftControl, float& rightControl)
{
    if (msMode_)
    {
        // In M/S mode, process mid and side independently
        // Convert L/R levels to M/S levels
        float midLevel = (leftLevel + rightLevel) * 0.5f;
        float sideLevel = (leftLevel - rightLevel) * 0.5f;
        
        // Apply stereo linking to M/S levels
        float linkedMidLevel = midLevel;
        float linkedSideLevel = sideLevel;
        
        if (linkAmount_ > 0.0f)
        {
            // Link mid and side levels based on link amount
            float maxLevel = std::max(midLevel, sideLevel);
            linkedMidLevel = lerp(midLevel, maxLevel, linkAmount_);
            linkedSideLevel = lerp(sideLevel, maxLevel, linkAmount_);
        }
        
        // Convert back to L/R control levels
        leftControl = linkedMidLevel + linkedSideLevel;
        rightControl = linkedMidLevel - linkedSideLevel;
    }
    else
    {
        // In L/R mode, apply stereo linking
        if (linkAmount_ <= 0.0f)
        {
            // Independent processing
            leftControl = leftLevel;
            rightControl = rightLevel;
        }
        else
        {
            // Calculate linked level (use maximum of both channels)
            float linkedLevel = std::max(leftLevel, rightLevel);
            
            // Interpolate between independent and linked levels
            leftControl = lerp(leftLevel, linkedLevel, linkAmount_);
            rightControl = lerp(rightLevel, linkedLevel, linkAmount_);
        }
    }
}

void StereoLink::processAudio(float leftInput, float rightInput, 
                             float& leftOutput, float& rightOutput)
{
    if (msMode_)
    {
        if (isEncoding_)
        {
            // Encode L/R to M/S
            float mid, side;
            encodeMS(leftInput, rightInput, mid, side);
            leftOutput = mid;
            rightOutput = side;
        }
        else
        {
            // Decode M/S to L/R
            decodeMS(leftInput, rightInput, leftOutput, rightOutput);
        }
    }
    else
    {
        // Pass through unchanged in L/R mode
        leftOutput = leftInput;
        rightOutput = rightInput;
    }
}

void StereoLink::processDetectionBlock(const float* leftLevels, const float* rightLevels,
                                      float* leftControls, float* rightControls, int numSamples)
{
    for (int i = 0; i < numSamples; ++i)
    {
        processDetectionLevels(leftLevels[i], rightLevels[i], 
                              leftControls[i], rightControls[i]);
    }
}

void StereoLink::processAudioBlock(const float* leftInputs, const float* rightInputs,
                                  float* leftOutputs, float* rightOutputs, int numSamples)
{
    for (int i = 0; i < numSamples; ++i)
    {
        processAudio(leftInputs[i], rightInputs[i], 
                    leftOutputs[i], rightOutputs[i]);
    }
}

void StereoLink::reset()
{
    // No state to reset for this processor
}

void StereoLink::setLinkAmount(float linkAmount)
{
    linkAmount_ = clampLinkAmount(linkAmount);
}

void StereoLink::setMSMode(bool msMode)
{
    msMode_ = msMode;
}

float StereoLink::clampLinkAmount(float value) const
{
    return std::max(0.0f, std::min(1.0f, value));
}

float StereoLink::lerp(float a, float b, float t) const
{
    t = std::max(0.0f, std::min(1.0f, t));
    return a + t * (b - a);
}

float StereoLink::dbToLinear(float dbValue) const
{
    return std::pow(10.0f, dbValue / 20.0f);
}

float StereoLink::linearToDb(float linearValue) const
{
    const float minLevel = 1e-6f;  // -120 dBFS
    linearValue = std::max(linearValue, minLevel);
    return 20.0f * std::log10(linearValue);
}

void StereoLink::encodeMS(float left, float right, float& mid, float& side)
{
    mid = (left + right) * 0.5f;
    side = (left - right) * 0.5f;
}

void StereoLink::decodeMS(float mid, float side, float& left, float& right)
{
    left = mid + side;
    right = mid - side;
}

} // namespace EzSqueeze::DSP
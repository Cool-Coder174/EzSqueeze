#include "ProgramDependentRelease.h"
#include <algorithm>
#include <cmath>

namespace EzSqueeze::DSP {

ProgramDependentRelease::ProgramDependentRelease(float baseReleaseMs, 
                                                 float fastReleaseMs, 
                                                 float slowReleaseMs)
    : baseReleaseMs_(0.0f)
    , fastReleaseMs_(0.0f)
    , slowReleaseMs_(0.0f)
    , sensitivity_(0.5f)
    , sampleRate_(44100.0f)
    , currentReleaseMs_(0.0f)
    , prevInputLevel_(0.0f)
    , transientDetector_(0.0f)
    , sustainedDetector_(0.0f)
    , transientAlpha_(TRANSIENT_ALPHA)
    , sustainedAlpha_(SUSTAINED_ALPHA)
{
    prepare(baseReleaseMs, fastReleaseMs, slowReleaseMs, 44100.0f);
}

void ProgramDependentRelease::prepare(float baseReleaseMs, float fastReleaseMs, 
                                     float slowReleaseMs, float sampleRate)
{
    baseReleaseMs_ = clampTime(baseReleaseMs);
    fastReleaseMs_ = clampTime(fastReleaseMs);
    slowReleaseMs_ = clampTime(slowReleaseMs);
    sampleRate_ = sampleRate;
    
    // Reset state
    reset();
}

float ProgramDependentRelease::processSample(float inputLevel, float gainReduction, float /*prevInputLevel*/)
{
    // Update detectors
    updateTransientDetector(inputLevel);
    updateSustainedDetector(inputLevel);
    
    // Calculate adaptive release time
    currentReleaseMs_ = calculateAdaptiveRelease(inputLevel, gainReduction);
    
    // Store current level for next iteration
    prevInputLevel_ = inputLevel;
    
    return currentReleaseMs_;
}

void ProgramDependentRelease::processBlock(const float* inputLevels, const float* gainReductions,
                                          float* releaseTimes, int numSamples)
{
    for (int i = 0; i < numSamples; ++i)
    {
        float prevLevel = (i > 0) ? inputLevels[i - 1] : prevInputLevel_;
        releaseTimes[i] = processSample(inputLevels[i], gainReductions[i], prevLevel);
    }
}

void ProgramDependentRelease::reset()
{
    prevInputLevel_ = 0.0f;
    transientDetector_ = 0.0f;
    sustainedDetector_ = 0.0f;
    currentReleaseMs_ = baseReleaseMs_;
}

void ProgramDependentRelease::setBaseReleaseTime(float releaseMs)
{
    baseReleaseMs_ = clampTime(releaseMs);
}

void ProgramDependentRelease::setFastReleaseTime(float releaseMs)
{
    fastReleaseMs_ = clampTime(releaseMs);
}

void ProgramDependentRelease::setSlowReleaseTime(float releaseMs)
{
    slowReleaseMs_ = clampTime(releaseMs);
}

void ProgramDependentRelease::setSensitivity(float sensitivity)
{
    sensitivity_ = clampSensitivity(sensitivity);
}

bool ProgramDependentRelease::detectTransient(float inputLevel, float gainReduction) const
{
    // Detect transients based on:
    // 1. Large level change
    // 2. Significant gain reduction
    
    float levelChange = std::abs(inputLevel - prevInputLevel_);
    bool levelTransient = levelChange > TRANSIENT_THRESHOLD;
    bool grTransient = std::abs(gainReduction) > GAIN_REDUCTION_THRESHOLD;
    
    return levelTransient && grTransient;
}

void ProgramDependentRelease::updateTransientDetector(float inputLevel)
{
    // Update transient detector with fast response
    float levelChange = std::abs(inputLevel - prevInputLevel_);
    transientDetector_ = (1.0f - transientAlpha_) * transientDetector_ + 
                        transientAlpha_ * levelChange;
}

void ProgramDependentRelease::updateSustainedDetector(float inputLevel)
{
    // Update sustained material detector with slow response
    float levelChange = std::abs(inputLevel - prevInputLevel_);
    sustainedDetector_ = (1.0f - sustainedAlpha_) * sustainedDetector_ + 
                        sustainedAlpha_ * levelChange;
}

float ProgramDependentRelease::calculateAdaptiveRelease(float inputLevel, float gainReduction) const
{
    // Determine if we're dealing with transient or sustained material
    bool isTransient = detectTransient(inputLevel, gainReduction);
    
    // Calculate adaptive release time based on material type
    float adaptiveRelease;
    
    if (isTransient)
    {
        // Use fast release for transients to prevent pumping
        adaptiveRelease = fastReleaseMs_;
    }
    else
    {
        // Use slow release for sustained material for smooth compression
        adaptiveRelease = slowReleaseMs_;
    }
    
    // Apply sensitivity factor to blend between base and adaptive release
    float sensitivityFactor = sensitivity_;
    adaptiveRelease = baseReleaseMs_ + sensitivityFactor * (adaptiveRelease - baseReleaseMs_);
    
    // Ensure we don't go below minimum release time
    return std::max(1.0f, adaptiveRelease);
}

float ProgramDependentRelease::clampTime(float timeMs) const
{
    return std::max(0.1f, std::min(1000.0f, timeMs));
}

float ProgramDependentRelease::clampSensitivity(float sens) const
{
    return std::max(0.0f, std::min(1.0f, sens));
}

} // namespace EzSqueeze::DSP
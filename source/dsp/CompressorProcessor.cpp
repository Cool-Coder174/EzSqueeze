#include "CompressorProcessor.h"
#include <algorithm>
#include <cmath>

namespace EzSqueeze::DSP {

CompressorProcessor::CompressorProcessor(float sampleRate, int maxBlockSize)
    : detector_(sampleRate, DetectorEngine::Mode::Peak)
    , gainComputer_(-18.0f, 4.0f, GainComputer::KneeType::Medium)
    , lookaheadBuffer_(5.0f, sampleRate)
    , stereoLink_(0.0f, false)
    , sidechainFilter_(sampleRate, 0.0f, 0.0f)
    , envelopeFollower_(sampleRate, 1.0f, 100.0f)
    , autoMakeupGain_(AutoMakeupGain::Mode::Off, 0.0f)
    , programDependentRelease_(100.0f, 50.0f, 300.0f)
    , threshold_(-18.0f)
    , ratio_(4.0f)
    , attackMs_(1.0f)
    , releaseMs_(100.0f)
    , knee_(GainComputer::KneeType::Medium)
    , lookaheadMs_(0.0f)
    , stereoLinkAmount_(0.0f)
    , msMode_(false)
    , hpfFreq_(0.0f)
    , lpfFreq_(0.0f)
    , mix_(1.0f)
    , makeupGain_(0.0f)
    , autoMakeup_(false)
    , programDependentReleaseEnabled_(false)
    , inputLevel_(0.0f)
    , outputLevel_(0.0f)
    , gainReduction_(0.0f)
    , sampleRate_(sampleRate)
    , maxBlockSize_(maxBlockSize)
{
    prepare(sampleRate, maxBlockSize);
}

void CompressorProcessor::prepare(float sampleRate, int maxBlockSize)
{
    sampleRate_ = sampleRate;
    maxBlockSize_ = maxBlockSize;
    
    // Prepare all DSP components
    detector_.prepare(sampleRate, DetectorEngine::Mode::Peak, 5.0f);
    gainComputer_.setParameters(threshold_, ratio_, knee_);
    lookaheadBuffer_.prepare(sampleRate, 10.0f);
    stereoLink_.prepare(stereoLinkAmount_, msMode_);
    sidechainFilter_.prepare(sampleRate, hpfFreq_, lpfFreq_);
    envelopeFollower_.prepare(sampleRate, attackMs_, releaseMs_);
    autoMakeupGain_.prepare(autoMakeup_ ? AutoMakeupGain::Mode::Adaptive : AutoMakeupGain::Mode::Off, 
                           makeupGain_, threshold_, ratio_);
    programDependentRelease_.prepare(100.0f, 50.0f, 300.0f, sampleRate);
    
    // Reset all state
    reset();
}

void CompressorProcessor::processStereo(const float* leftInput, const float* rightInput,
                                       float* leftOutput, float* rightOutput, int numSamples)
{
    // Clamp block size
    numSamples = std::min(numSamples, maxBlockSize_);
    
    // Process detection and gain reduction
    processDetectionStereo(leftInput, rightInput, numSamples);
    
    // Apply gain reduction to audio
    applyGainReduction(leftInput, rightInput, leftOutput, rightOutput, numSamples);
    
    // Update metering
    updateMetering(leftInput, rightInput, leftOutput, rightOutput, numSamples);
}

void CompressorProcessor::processMono(const float* input, float* output, int numSamples)
{
    // Clamp block size
    numSamples = std::min(numSamples, maxBlockSize_);
    
    // Process detection and gain reduction
    processDetectionMono(input, numSamples);
    
    // Apply gain reduction to audio (mono)
    for (int i = 0; i < numSamples; ++i)
    {
        float gainLinear = std::pow(10.0f, leftEnvelope_[i] / 20.0f);
        float compressed = input[i] * gainLinear;
        
        // Apply parallel mix
        float mixed = mix_ * compressed + (1.0f - mix_) * input[i];
        
        // Apply makeup gain
        output[i] = autoMakeupGain_.processSample(mixed, leftGainReduction_[i]);
    }
    
    // Update metering
    updateMetering(input, input, output, output, numSamples);
}

void CompressorProcessor::reset()
{
    detector_.reset();
    lookaheadBuffer_.reset();
    stereoLink_.reset();
    sidechainFilter_.reset();
    envelopeFollower_.reset();
    autoMakeupGain_.reset();
    programDependentRelease_.reset();
    
    // Clear buffers
    std::fill(leftBuffer_.begin(), leftBuffer_.end(), 0.0f);
    std::fill(rightBuffer_.begin(), rightBuffer_.end(), 0.0f);
    std::fill(leftDelayed_.begin(), leftDelayed_.end(), 0.0f);
    std::fill(rightDelayed_.begin(), rightDelayed_.end(), 0.0f);
    std::fill(leftLevels_.begin(), leftLevels_.end(), 0.0f);
    std::fill(rightLevels_.begin(), rightLevels_.end(), 0.0f);
    std::fill(leftControls_.begin(), leftControls_.end(), 0.0f);
    std::fill(rightControls_.begin(), rightControls_.end(), 0.0f);
    std::fill(leftGainReduction_.begin(), leftGainReduction_.end(), 0.0f);
    std::fill(rightGainReduction_.begin(), rightGainReduction_.end(), 0.0f);
    std::fill(leftEnvelope_.begin(), leftEnvelope_.end(), 0.0f);
    std::fill(rightEnvelope_.begin(), rightEnvelope_.end(), 0.0f);
    std::fill(leftCompressed_.begin(), leftCompressed_.end(), 0.0f);
    std::fill(rightCompressed_.begin(), rightCompressed_.end(), 0.0f);
    
    inputLevel_ = 0.0f;
    outputLevel_ = 0.0f;
    gainReduction_ = 0.0f;
}

void CompressorProcessor::setThreshold(float threshold)
{
    threshold_ = threshold;
    updateParameters();
}

void CompressorProcessor::setRatio(float ratio)
{
    ratio_ = std::max(1.0f, ratio);
    updateParameters();
}

void CompressorProcessor::setAttack(float attackMs)
{
    attackMs_ = std::max(0.1f, attackMs);
    updateParameters();
}

void CompressorProcessor::setRelease(float releaseMs)
{
    releaseMs_ = std::max(0.1f, releaseMs);
    updateParameters();
}

void CompressorProcessor::setKnee(GainComputer::KneeType knee)
{
    knee_ = knee;
    updateParameters();
}

void CompressorProcessor::setLookahead(float lookaheadMs)
{
    lookaheadMs_ = std::max(0.0f, std::min(10.0f, lookaheadMs));
    lookaheadBuffer_.setDelay(lookaheadMs_);
}

void CompressorProcessor::setStereoLink(float linkAmount)
{
    stereoLinkAmount_ = std::max(0.0f, std::min(1.0f, linkAmount));
    stereoLink_.setLinkAmount(stereoLinkAmount_);
}

void CompressorProcessor::setMSMode(bool msMode)
{
    msMode_ = msMode;
    stereoLink_.setMSMode(msMode_);
}

void CompressorProcessor::setHPFFrequency(float freq)
{
    hpfFreq_ = std::max(0.0f, freq);
    sidechainFilter_.setHPFFrequency(hpfFreq_);
}

void CompressorProcessor::setLPFFrequency(float freq)
{
    lpfFreq_ = std::max(0.0f, freq);
    sidechainFilter_.setLPFFrequency(lpfFreq_);
}

void CompressorProcessor::setMix(float mix)
{
    mix_ = std::max(0.0f, std::min(1.0f, mix));
}

void CompressorProcessor::setMakeupGain(float gain)
{
    makeupGain_ = std::max(-60.0f, std::min(60.0f, gain));
    autoMakeupGain_.setManualGain(makeupGain_);
}

void CompressorProcessor::setAutoMakeup(bool enabled)
{
    autoMakeup_ = enabled;
    autoMakeupGain_.setMode(enabled ? AutoMakeupGain::Mode::Adaptive : AutoMakeupGain::Mode::Off);
}

void CompressorProcessor::setProgramDependentRelease(bool enabled)
{
    programDependentReleaseEnabled_ = enabled;
}

int CompressorProcessor::getLatencySamples() const
{
    return lookaheadBuffer_.getDelaySamples();
}

void CompressorProcessor::updateParameters()
{
    gainComputer_.setParameters(threshold_, ratio_, knee_);
    envelopeFollower_.setAttackTime(attackMs_);
    envelopeFollower_.setReleaseTime(releaseMs_);
    autoMakeupGain_.updateCompressionParams(threshold_, ratio_);
}

void CompressorProcessor::processDetectionStereo(const float* leftInput, const float* rightInput, int numSamples)
{
    // Apply lookahead delay to audio
    lookaheadBuffer_.processBlock(leftInput, leftDelayed_.data(), numSamples);
    lookaheadBuffer_.processBlock(rightInput, rightDelayed_.data(), numSamples);
    
    // Detect levels
    detector_.processBlock(leftDelayed_.data(), leftLevels_.data(), numSamples);
    detector_.processBlock(rightDelayed_.data(), rightLevels_.data(), numSamples);
    
    // Apply sidechain filtering
    if (sidechainFilter_.isHPFEnabled() || sidechainFilter_.isLPFEnabled())
    {
        sidechainFilter_.processBlock(leftLevels_.data(), leftLevels_.data(), numSamples);
        sidechainFilter_.processBlock(rightLevels_.data(), rightLevels_.data(), numSamples);
    }
    
    // Apply stereo linking
    stereoLink_.processDetectionBlock(leftLevels_.data(), rightLevels_.data(),
                                     leftControls_.data(), rightControls_.data(), numSamples);
    
    // Calculate gain reduction
    gainComputer_.processBlock(leftControls_.data(), leftGainReduction_.data(), numSamples);
    gainComputer_.processBlock(rightControls_.data(), rightGainReduction_.data(), numSamples);
    
    // Apply envelope following
    if (programDependentReleaseEnabled_)
    {
        // Use program-dependent release
        programDependentRelease_.processBlock(leftControls_.data(), leftGainReduction_.data(),
                                             leftEnvelope_.data(), numSamples);
        programDependentRelease_.processBlock(rightControls_.data(), rightGainReduction_.data(),
                                             rightEnvelope_.data(), numSamples);
        
        // Apply envelope smoothing
        envelopeFollower_.processBlock(leftEnvelope_.data(), leftEnvelope_.data(), numSamples);
        envelopeFollower_.processBlock(rightEnvelope_.data(), rightEnvelope_.data(), numSamples);
    }
    else
    {
        // Use standard envelope following
        envelopeFollower_.processBlock(leftGainReduction_.data(), leftEnvelope_.data(), numSamples);
        envelopeFollower_.processBlock(rightGainReduction_.data(), rightEnvelope_.data(), numSamples);
    }
}

void CompressorProcessor::processDetectionMono(const float* input, int numSamples)
{
    // Apply lookahead delay
    lookaheadBuffer_.processBlock(input, leftDelayed_.data(), numSamples);
    
    // Detect level
    detector_.processBlock(leftDelayed_.data(), leftLevels_.data(), numSamples);
    
    // Apply sidechain filtering
    if (sidechainFilter_.isHPFEnabled() || sidechainFilter_.isLPFEnabled())
    {
        sidechainFilter_.processBlock(leftLevels_.data(), leftLevels_.data(), numSamples);
    }
    
    // Calculate gain reduction
    gainComputer_.processBlock(leftLevels_.data(), leftGainReduction_.data(), numSamples);
    
    // Apply envelope following
    if (programDependentReleaseEnabled_)
    {
        programDependentRelease_.processBlock(leftLevels_.data(), leftGainReduction_.data(),
                                             leftEnvelope_.data(), numSamples);
        envelopeFollower_.processBlock(leftEnvelope_.data(), leftEnvelope_.data(), numSamples);
    }
    else
    {
        envelopeFollower_.processBlock(leftGainReduction_.data(), leftEnvelope_.data(), numSamples);
    }
}

void CompressorProcessor::applyGainReduction(const float* leftInput, const float* rightInput,
                                           float* leftOutput, float* rightOutput, int numSamples)
{
    for (int i = 0; i < numSamples; ++i)
    {
        // Apply gain reduction (leftEnvelope_ contains gain reduction in dB, which is negative)
        float leftGainLinear = std::pow(10.0f, leftEnvelope_[i] / 20.0f);
        float rightGainLinear = std::pow(10.0f, rightEnvelope_[i] / 20.0f);
        
        leftCompressed_[i] = leftInput[i] * leftGainLinear;
        rightCompressed_[i] = rightInput[i] * rightGainLinear;
        
        // Apply parallel mix
        float leftMix = mix_ * leftCompressed_[i] + (1.0f - mix_) * leftInput[i];
        float rightMix = mix_ * rightCompressed_[i] + (1.0f - mix_) * rightInput[i];
        
        // Apply makeup gain (pass the gain reduction value, not the envelope)
        leftOutput[i] = autoMakeupGain_.processSample(leftMix, leftGainReduction_[i]);
        rightOutput[i] = autoMakeupGain_.processSample(rightMix, rightGainReduction_[i]);
    }
}

void CompressorProcessor::updateMetering(const float* leftInput, const float* rightInput,
                                        const float* leftOutput, const float* rightOutput, int numSamples)
{
    // Calculate RMS levels for metering
    float leftInputRMS = 0.0f, rightInputRMS = 0.0f;
    float leftOutputRMS = 0.0f, rightOutputRMS = 0.0f;
    
    for (int i = 0; i < numSamples; ++i)
    {
        leftInputRMS += leftInput[i] * leftInput[i];
        rightInputRMS += rightInput[i] * rightInput[i];
        leftOutputRMS += leftOutput[i] * leftOutput[i];
        rightOutputRMS += rightOutput[i] * rightOutput[i];
    }
    
    leftInputRMS = std::sqrt(leftInputRMS / numSamples);
    rightInputRMS = std::sqrt(rightInputRMS / numSamples);
    leftOutputRMS = std::sqrt(leftOutputRMS / numSamples);
    rightOutputRMS = std::sqrt(rightOutputRMS / numSamples);
    
    // Convert to dBFS
    inputLevel_ = 20.0f * std::log10(std::max(leftInputRMS, rightInputRMS));
    outputLevel_ = 20.0f * std::log10(std::max(leftOutputRMS, rightOutputRMS));
    
    // Calculate average gain reduction
    float avgGR = 0.0f;
    for (int i = 0; i < numSamples; ++i)
    {
        avgGR += leftGainReduction_[i];
    }
    gainReduction_ = avgGR / numSamples;
}

} // namespace EzSqueeze::DSP
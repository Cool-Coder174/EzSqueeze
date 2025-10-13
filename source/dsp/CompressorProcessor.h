#pragma once

#include "Detector.h"
#include "GainComputer.h"
#include "LookaheadBuffer.h"
#include "StereoLink.h"
#include "SidechainFilter.h"
#include "EnvelopeFollower.h"
#include "AutoMakeupGain.h"
#include "ProgramDependentRelease.h"

#include <array>
#include <memory>

namespace EzSqueeze::DSP {

/**
 * @brief Main compressor processor that integrates all DSP components
 * 
 * This class orchestrates the complete compression chain including
 * detection, gain computation, envelope following, and makeup gain.
 */
class CompressorProcessor
{
public:
    /**
     * @brief Constructor
     * @param sampleRate Sample rate in Hz
     * @param maxBlockSize Maximum block size for processing
     */
    CompressorProcessor(float sampleRate = 44100.0f, int maxBlockSize = 1024);

    /**
     * @brief Prepare the processor for audio processing
     * @param sampleRate New sample rate
     * @param maxBlockSize Maximum block size
     */
    void prepare(float sampleRate, int maxBlockSize);

    /**
     * @brief Process stereo audio
     * @param leftInput Left input buffer
     * @param rightInput Right input buffer
     * @param leftOutput Left output buffer
     * @param rightOutput Right output buffer
     * @param numSamples Number of samples to process
     */
    void processStereo(const float* leftInput, const float* rightInput,
                      float* leftOutput, float* rightOutput, int numSamples);

    /**
     * @brief Process mono audio
     * @param input Input buffer
     * @param output Output buffer
     * @param numSamples Number of samples to process
     */
    void processMono(const float* input, float* output, int numSamples);

    /**
     * @brief Reset all processor state
     */
    void reset();

    // Parameter setters
    void setThreshold(float threshold);
    void setRatio(float ratio);
    void setAttack(float attackMs);
    void setRelease(float releaseMs);
    void setKnee(GainComputer::KneeType knee);
    void setLookahead(float lookaheadMs);
    void setStereoLink(float linkAmount);
    void setMSMode(bool msMode);
    void setHPFFrequency(float freq);
    void setLPFFrequency(float freq);
    void setMix(float mix);
    void setMakeupGain(float gain);
    void setAutoMakeup(bool enabled);
    void setProgramDependentRelease(bool enabled);

    // Parameter getters
    float getThreshold() const { return threshold_; }
    float getRatio() const { return ratio_; }
    float getAttack() const { return attackMs_; }
    float getRelease() const { return releaseMs_; }
    GainComputer::KneeType getKnee() const { return knee_; }
    float getLookahead() const { return lookaheadMs_; }
    float getStereoLink() const { return stereoLinkAmount_; }
    bool getMSMode() const { return msMode_; }
    float getHPFFrequency() const { return hpfFreq_; }
    float getLPFFrequency() const { return lpfFreq_; }
    float getMix() const { return mix_; }
    float getMakeupGain() const { return makeupGain_; }
    bool getAutoMakeup() const { return autoMakeup_; }
    bool getProgramDependentRelease() const { return programDependentReleaseEnabled_; }

    // Metering
    float getInputLevel() const { return inputLevel_; }
    float getOutputLevel() const { return outputLevel_; }
    float getGainReduction() const { return gainReduction_; }
    int getLatencySamples() const;

private:
    // DSP components
    DetectorEngine detector_;
    GainComputer gainComputer_;
    LookaheadBuffer lookaheadBuffer_;
    StereoLink stereoLink_;
    SidechainFilter sidechainFilter_;
    EnvelopeFollower envelopeFollower_;
    AutoMakeupGain autoMakeupGain_;
    ProgramDependentRelease programDependentRelease_;

    // Parameters
    float threshold_;
    float ratio_;
    float attackMs_;
    float releaseMs_;
    GainComputer::KneeType knee_;
    float lookaheadMs_;
    float stereoLinkAmount_;
    bool msMode_;
    float hpfFreq_;
    float lpfFreq_;
    float mix_;
    float makeupGain_;
    bool autoMakeup_;
    bool programDependentReleaseEnabled_;

    // Processing buffers
    std::array<float, 1024> leftBuffer_;
    std::array<float, 1024> rightBuffer_;
    std::array<float, 1024> leftDelayed_;
    std::array<float, 1024> rightDelayed_;
    std::array<float, 1024> leftLevels_;
    std::array<float, 1024> rightLevels_;
    std::array<float, 1024> leftControls_;
    std::array<float, 1024> rightControls_;
    std::array<float, 1024> leftGainReduction_;
    std::array<float, 1024> rightGainReduction_;
    std::array<float, 1024> leftEnvelope_;
    std::array<float, 1024> rightEnvelope_;
    std::array<float, 1024> leftCompressed_;
    std::array<float, 1024> rightCompressed_;

    // Metering
    float inputLevel_;
    float outputLevel_;
    float gainReduction_;

    // State
    float sampleRate_;
    int maxBlockSize_;

    /**
     * @brief Update all DSP component parameters
     */
    void updateParameters();

    /**
     * @brief Process detection and gain reduction for stereo
     * @param leftInput Left input buffer
     * @param rightInput Right input buffer
     * @param numSamples Number of samples
     */
    void processDetectionStereo(const float* leftInput, const float* rightInput, int numSamples);

    /**
     * @brief Process detection and gain reduction for mono
     * @param input Input buffer
     * @param numSamples Number of samples
     */
    void processDetectionMono(const float* input, int numSamples);

    /**
     * @brief Apply gain reduction to audio
     * @param leftInput Left input buffer
     * @param rightInput Right input buffer
     * @param leftOutput Left output buffer
     * @param rightOutput Right output buffer
     * @param numSamples Number of samples
     */
    void applyGainReduction(const float* leftInput, const float* rightInput,
                           float* leftOutput, float* rightOutput, int numSamples);

    /**
     * @brief Update metering values
     * @param leftInput Left input buffer
     * @param rightInput Right input buffer
     * @param leftOutput Left output buffer
     * @param rightOutput Right output buffer
     * @param numSamples Number of samples
     */
    void updateMetering(const float* leftInput, const float* rightInput,
                       const float* leftOutput, const float* rightOutput, int numSamples);
};

} // namespace EzSqueeze::DSP
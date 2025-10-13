#pragma once

#include <cmath>
#include <algorithm>

namespace EzSqueeze::DSP {

/**
 * @brief Stereo linking and M/S processing for stereo compression
 * 
 * Provides stereo linking (0-100%) and M/S encoding/decoding for
 * independent mid and side channel processing.
 */
class StereoLink
{
public:
    /**
     * @brief Constructor
     * @param linkAmount Stereo link amount (0.0 = independent, 1.0 = fully linked)
     * @param msMode True for M/S processing, false for L/R processing
     */
    StereoLink(float linkAmount = 0.0f, bool msMode = false);

    /**
     * @brief Prepare the processor
     * @param linkAmount Stereo link amount (0.0 to 1.0)
     * @param msMode True for M/S processing, false for L/R processing
     */
    void prepare(float linkAmount, bool msMode);

    /**
     * @brief Process stereo detection levels
     * @param leftLevel Left channel level in dBFS
     * @param rightLevel Right channel level in dBFS
     * @param leftControl Output left control level in dBFS
     * @param rightControl Output right control level in dBFS
     */
    void processDetectionLevels(float leftLevel, float rightLevel, 
                               float& leftControl, float& rightControl);

    /**
     * @brief Process stereo audio for M/S encoding/decoding
     * @param leftInput Left input sample
     * @param rightInput Right input sample
     * @param leftOutput Left output sample
     * @param rightOutput Right output sample
     */
    void processAudio(float leftInput, float rightInput, 
                     float& leftOutput, float& rightOutput);

    /**
     * @brief Process a block of stereo detection levels
     * @param leftLevels Left channel levels in dBFS
     * @param rightLevels Right channel levels in dBFS
     * @param leftControls Output left control levels in dBFS
     * @param rightControls Output right control levels in dBFS
     * @param numSamples Number of samples to process
     */
    void processDetectionBlock(const float* leftLevels, const float* rightLevels,
                              float* leftControls, float* rightControls, int numSamples);

    /**
     * @brief Process a block of stereo audio
     * @param leftInputs Left input samples
     * @param rightInputs Right input samples
     * @param leftOutputs Left output samples
     * @param rightOutputs Right output samples
     * @param numSamples Number of samples to process
     */
    void processAudioBlock(const float* leftInputs, const float* rightInputs,
                          float* leftOutputs, float* rightOutputs, int numSamples);

    /**
     * @brief Reset the processor state
     */
    void reset();

    /**
     * @brief Get current link amount
     * @return Link amount (0.0 to 1.0)
     */
    float getLinkAmount() const { return linkAmount_; }

    /**
     * @brief Get current M/S mode
     * @return True if in M/S mode
     */
    bool getMSMode() const { return msMode_; }

    /**
     * @brief Set link amount
     * @param linkAmount New link amount (0.0 to 1.0)
     */
    void setLinkAmount(float linkAmount);

    /**
     * @brief Set M/S mode
     * @param msMode True for M/S mode, false for L/R mode
     */
    void setMSMode(bool msMode);

private:
    float linkAmount_;
    bool msMode_;
    bool isEncoding_;  // True when encoding L/R to M/S, false when decoding

    /**
     * @brief Clamp value between 0.0 and 1.0
     * @param value Value to clamp
     * @return Clamped value
     */
    float clampLinkAmount(float value) const;

    /**
     * @brief Linear interpolation between two values
     * @param a Start value
     * @param b End value
     * @param t Interpolation factor (0.0 to 1.0)
     * @return Interpolated value
     */
    float lerp(float a, float b, float t) const;

    /**
     * @brief Convert dB to linear scale
     * @param dbValue Value in dB
     * @return Value in linear scale
     */
    float dbToLinear(float dbValue) const;

    /**
     * @brief Convert linear scale to dB
     * @param linearValue Value in linear scale
     * @return Value in dB
     */
    float linearToDb(float linearValue) const;

    /**
     * @brief Encode L/R to M/S
     * @param left Left channel sample
     * @param right Right channel sample
     * @param mid Mid channel output
     * @param side Side channel output
     */
    void encodeMS(float left, float right, float& mid, float& side);

    /**
     * @brief Decode M/S to L/R
     * @param mid Mid channel sample
     * @param side Side channel sample
     * @param left Left channel output
     * @param right Right channel output
     */
    void decodeMS(float mid, float side, float& left, float& right);
};

} // namespace EzSqueeze::DSP
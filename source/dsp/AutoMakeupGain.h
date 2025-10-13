#pragma once

#include <cmath>
#include <algorithm>

namespace EzSqueeze::DSP {

/**
 * @brief Auto-makeup gain for loudness compensation
 * 
 * Automatically calculates makeup gain to compensate for gain reduction
 * and maintain perceived loudness. Supports both static and adaptive modes.
 */
class AutoMakeupGain
{
public:
    /**
     * @brief Makeup gain modes
     */
    enum class Mode
    {
        Off,        ///< Manual makeup gain only
        Static,     ///< Calculate based on threshold and ratio
        Adaptive    ///< Measure actual gain reduction and adapt
    };

    /**
     * @brief Constructor
     * @param mode Makeup gain mode
     * @param manualGain Manual makeup gain in dB
     */
    AutoMakeupGain(Mode mode = Mode::Off, float manualGain = 0.0f);

    /**
     * @brief Prepare the auto-makeup gain
     * @param mode Makeup gain mode
     * @param manualGain Manual makeup gain in dB
     * @param threshold Compression threshold in dBFS
     * @param ratio Compression ratio
     */
    void prepare(Mode mode, float manualGain, float threshold, float ratio);

    /**
     * @brief Process a single sample
     * @param input Input sample
     * @param gainReduction Current gain reduction in dB (negative value)
     * @return Output sample with makeup gain applied
     */
    float processSample(float input, float gainReduction);

    /**
     * @brief Process a block of samples
     * @param input Input buffer
     * @param output Output buffer
     * @param gainReduction Gain reduction buffer in dB (negative values)
     * @param numSamples Number of samples to process
     */
    void processBlock(const float* input, float* output, 
                     const float* gainReduction, int numSamples);

    /**
     * @brief Update compression parameters (for static mode)
     * @param threshold New threshold in dBFS
     * @param ratio New compression ratio
     */
    void updateCompressionParams(float threshold, float ratio);

    /**
     * @brief Set manual makeup gain
     * @param gain Makeup gain in dB
     */
    void setManualGain(float gain);

    /**
     * @brief Set makeup gain mode
     * @param mode New mode
     */
    void setMode(Mode mode);

    /**
     * @brief Reset the adaptive state
     */
    void reset();

    /**
     * @brief Get current makeup gain
     * @return Makeup gain in dB
     */
    float getMakeupGain() const { return currentMakeupGain_; }

    /**
     * @brief Get current mode
     * @return Current mode
     */
    Mode getMode() const { return mode_; }

    /**
     * @brief Get manual gain
     * @return Manual gain in dB
     */
    float getManualGain() const { return manualGain_; }

    /**
     * @brief Get adaptive gain (only valid in adaptive mode)
     * @return Adaptive gain in dB
     */
    float getAdaptiveGain() const { return adaptiveGain_; }

private:
    Mode mode_;
    float manualGain_;
    float threshold_;
    float ratio_;
    float currentMakeupGain_;
    float adaptiveGain_;
    float runningAverageGR_;
    float alpha_;  // Smoothing coefficient for adaptive mode

    // Constants
    static constexpr float ADAPTIVE_ALPHA = 0.001f;  // Smoothing for adaptive gain
    static constexpr float COMPENSATION_FACTOR = 0.7f;  // 70% compensation to avoid over-compensation
    static constexpr float ASSUMED_OVERSHOOT = 10.0f;  // Typical material overshoot in dB

    /**
     * @brief Calculate static makeup gain based on threshold and ratio
     * @param threshold Compression threshold in dBFS
     * @param ratio Compression ratio
     * @return Calculated makeup gain in dB
     */
    float calculateStaticMakeup(float threshold, float ratio) const;

    /**
     * @brief Update adaptive makeup gain
     * @param gainReduction Current gain reduction in dB
     */
    void updateAdaptiveGain(float gainReduction);

    /**
     * @brief Clamp gain value to reasonable range
     * @param gain Gain in dB
     * @return Clamped gain
     */
    float clampGain(float gain) const;
};

} // namespace EzSqueeze::DSP
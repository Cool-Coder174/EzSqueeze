#pragma once

#include <cmath>
#include <algorithm>

namespace EzSqueeze::DSP {

/**
 * @brief Program-dependent release for intelligent compression behavior
 * 
 * Automatically adjusts release time based on input signal characteristics
 * to prevent pumping and maintain musical compression.
 */
class ProgramDependentRelease
{
public:
    /**
     * @brief Constructor
     * @param baseReleaseMs Base release time in milliseconds
     * @param fastReleaseMs Fast release time for transients in milliseconds
     * @param slowReleaseMs Slow release time for sustained material in milliseconds
     */
    ProgramDependentRelease(float baseReleaseMs = 100.0f, 
                           float fastReleaseMs = 50.0f, 
                           float slowReleaseMs = 300.0f);

    /**
     * @brief Prepare the program-dependent release
     * @param baseReleaseMs Base release time in milliseconds
     * @param fastReleaseMs Fast release time for transients in milliseconds
     * @param slowReleaseMs Slow release time for sustained material in milliseconds
     * @param sampleRate Sample rate in Hz
     */
    void prepare(float baseReleaseMs, float fastReleaseMs, float slowReleaseMs, float sampleRate);

    /**
     * @brief Process and return adaptive release time
     * @param inputLevel Current input level in dBFS
     * @param gainReduction Current gain reduction in dB
     * @param prevInputLevel Previous input level in dBFS
     * @return Adaptive release time in milliseconds
     */
    float processSample(float inputLevel, float gainReduction, float prevInputLevel);

    /**
     * @brief Process a block of samples
     * @param inputLevels Input levels in dBFS
     * @param gainReductions Gain reductions in dB
     * @param releaseTimes Output release times in milliseconds
     * @param numSamples Number of samples to process
     */
    void processBlock(const float* inputLevels, const float* gainReductions,
                     float* releaseTimes, int numSamples);

    /**
     * @brief Reset the processor state
     */
    void reset();

    /**
     * @brief Set base release time
     * @param releaseMs Release time in milliseconds
     */
    void setBaseReleaseTime(float releaseMs);

    /**
     * @brief Set fast release time
     * @param releaseMs Release time in milliseconds
     */
    void setFastReleaseTime(float releaseMs);

    /**
     * @brief Set slow release time
     * @param releaseMs Release time in milliseconds
     */
    void setSlowReleaseTime(float releaseMs);

    /**
     * @brief Set sensitivity for transient detection
     * @param sensitivity Sensitivity factor (0.0 to 1.0)
     */
    void setSensitivity(float sensitivity);

    /**
     * @brief Get current base release time
     * @return Release time in milliseconds
     */
    float getBaseReleaseTime() const { return baseReleaseMs_; }

    /**
     * @brief Get current fast release time
     * @return Release time in milliseconds
     */
    float getFastReleaseTime() const { return fastReleaseMs_; }

    /**
     * @brief Get current slow release time
     * @return Release time in milliseconds
     */
    float getSlowReleaseTime() const { return slowReleaseMs_; }

    /**
     * @brief Get current sensitivity
     * @return Sensitivity factor
     */
    float getSensitivity() const { return sensitivity_; }

    /**
     * @brief Get current adaptive release time
     * @return Current release time in milliseconds
     */
    float getCurrentReleaseTime() const { return currentReleaseMs_; }

private:
    float baseReleaseMs_;
    float fastReleaseMs_;
    float slowReleaseMs_;
    float sensitivity_;
    float sampleRate_;
    float currentReleaseMs_;
    
    // Transient detection state
    float prevInputLevel_;
    float transientDetector_;
    float sustainedDetector_;
    
    // Smoothing coefficients
    float transientAlpha_;
    float sustainedAlpha_;

    // Constants
    static constexpr float TRANSIENT_THRESHOLD = 3.0f;  // dB change for transient detection
    static constexpr float GAIN_REDUCTION_THRESHOLD = 3.0f;  // dB GR for transient response
    static constexpr float TRANSIENT_ALPHA = 0.1f;  // Fast response for transients
    static constexpr float SUSTAINED_ALPHA = 0.01f;  // Slow response for sustained material

    /**
     * @brief Detect transients in the input signal
     * @param inputLevel Current input level
     * @param gainReduction Current gain reduction
     * @return True if transient detected
     */
    bool detectTransient(float inputLevel, float gainReduction) const;

    /**
     * @brief Update transient detector
     * @param inputLevel Current input level
     */
    void updateTransientDetector(float inputLevel);

    /**
     * @brief Update sustained material detector
     * @param inputLevel Current input level
     */
    void updateSustainedDetector(float inputLevel);

    /**
     * @brief Calculate adaptive release time
     * @param inputLevel Current input level
     * @param gainReduction Current gain reduction
     * @return Adaptive release time in milliseconds
     */
    float calculateAdaptiveRelease(float inputLevel, float gainReduction) const;

    /**
     * @brief Clamp time value to valid range
     * @param timeMs Time in milliseconds
     * @return Clamped time value
     */
    float clampTime(float timeMs) const;

    /**
     * @brief Clamp sensitivity to valid range
     * @param sens Sensitivity value
     * @return Clamped sensitivity value
     */
    float clampSensitivity(float sens) const;
};

} // namespace EzSqueeze::DSP
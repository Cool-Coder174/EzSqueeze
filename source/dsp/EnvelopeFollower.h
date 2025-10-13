#pragma once

#include <cmath>

namespace EzSqueeze::DSP {

/**
 * @brief Envelope follower for smoothing detection signals
 * 
 * Implements attack and release time constants with separate coefficients
 * for musical compression behavior. All processing is real-time safe.
 */
class EnvelopeFollower
{
public:
    /**
     * @brief Constructor
     * @param sampleRate Sample rate in Hz
     * @param attackMs Attack time in milliseconds
     * @param releaseMs Release time in milliseconds
     */
    EnvelopeFollower(float sampleRate = 44100.0f, float attackMs = 1.0f, float releaseMs = 100.0f);

    /**
     * @brief Prepare the envelope follower
     * @param sampleRate New sample rate
     * @param attackMs Attack time in milliseconds
     * @param releaseMs Release time in milliseconds
     */
    void prepare(float sampleRate, float attackMs, float releaseMs);

    /**
     * @brief Process a single sample
     * @param input Input detection level (typically in dBFS)
     * @return Smoothed envelope output
     */
    float processSample(float input);

    /**
     * @brief Process a block of samples
     * @param input Input buffer
     * @param output Output buffer
     * @param numSamples Number of samples to process
     */
    void processBlock(const float* input, float* output, int numSamples);

    /**
     * @brief Reset the envelope state
     */
    void reset();

    /**
     * @brief Set attack time
     * @param attackMs Attack time in milliseconds
     */
    void setAttackTime(float attackMs);

    /**
     * @brief Set release time
     * @param releaseMs Release time in milliseconds
     */
    void setReleaseTime(float releaseMs);

    /**
     * @brief Get current attack time
     * @return Attack time in milliseconds
     */
    float getAttackTime() const { return attackMs_; }

    /**
     * @brief Get current release time
     * @return Release time in milliseconds
     */
    float getReleaseTime() const { return releaseMs_; }

    /**
     * @brief Get current envelope level
     * @return Current envelope level
     */
    float getCurrentLevel() const { return envelopeLevel_; }

    /**
     * @brief Check if envelope is in attack phase
     * @return True if currently attacking
     */
    bool isAttacking() const { return isAttacking_; }

private:
    float sampleRate_;
    float attackMs_;
    float releaseMs_;
    float attackCoeff_;
    float releaseCoeff_;
    float envelopeLevel_;
    bool isAttacking_;

    /**
     * @brief Convert time constant to coefficient
     * @param timeMs Time constant in milliseconds
     * @return Filter coefficient
     */
    float timeToCoeff(float timeMs) const;

    /**
     * @brief Clamp time value to valid range
     * @param timeMs Time in milliseconds
     * @return Clamped time value
     */
    float clampTime(float timeMs) const;
};

} // namespace EzSqueeze::DSP
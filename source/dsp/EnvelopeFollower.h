/**
 * @file EnvelopeFollower.h
 * @brief Attack/Release envelope follower for compression
 * 
 * Implements smooth envelope following with separate attack
 * and release time constants for musical dynamics control.
 * 
 * @author Isaac Hernandez
 * @date October 2025
 */

#pragma once

#include <cmath>
#include <algorithm>

namespace EzSqueeze {
namespace DSP {

/**
 * @class EnvelopeFollower
 * @brief Smooth envelope with attack/release characteristics
 * 
 * The EnvelopeFollower smooths the gain reduction signal with
 * separate attack (rising) and release (falling) time constants,
 * creating the characteristic compression response.
 * 
 * RT-Safe: Yes (no allocations after prepare())
 * Complexity: O(1) per sample
 */
class EnvelopeFollower
{
public:
    EnvelopeFollower() = default;
    ~EnvelopeFollower() = default;

    /**
     * @brief Prepare envelope follower for processing
     * @param sampleRate Sample rate in Hz
     */
    void prepare(double sampleRate);

    /**
     * @brief Set attack time
     * @param attackMs Attack time in milliseconds (0.1 to 100ms)
     */
    void setAttack(float attackMs);

    /**
     * @brief Set release time
     * @param releaseMs Release time in milliseconds (10 to 1000ms)
     */
    void setRelease(float releaseMs);

    /**
     * @brief Process single sample through envelope
     * @param input Input gain reduction value (typically negative dB)
     * @return Smoothed gain reduction value
     */
    float processSample(float input);

    /**
     * @brief Reset envelope state
     */
    void reset();

    /**
     * @brief Get current envelope value
     * @return Current envelope state
     */
    float getCurrentValue() const { return m_envelope; }

private:
    double m_sampleRate = 48000.0;
    float m_envelope = 0.0f;
    
    float m_attackCoeff = 0.0f;
    float m_releaseCoeff = 0.0f;
    
    /**
     * @brief Convert time in ms to smoothing coefficient
     * @param timeMs Time constant in milliseconds
     * @return Alpha coefficient for one-pole filter
     */
    float timeToCoeff(float timeMs) const;
};

} // namespace DSP
} // namespace EzSqueeze

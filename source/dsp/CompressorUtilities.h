/**
 * @file CompressorUtilities.h
 * @brief Utility classes for compression processing
 * 
 * Includes parallel mix, auto-makeup gain, and program-dependent
 * release algorithms.
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
 * @class ParallelMix
 * @brief Parallel compression wet/dry blending
 * 
 * Blends between dry (uncompressed) and wet (compressed) signals
 * for New York-style parallel compression.
 * 
 * RT-Safe: Yes (pure computation)
 * Complexity: O(1)
 */
class ParallelMix
{
public:
    /**
     * @brief Set mix amount
     * @param mixPercent Mix percentage (0 = all dry, 100 = all wet)
     */
    void setMix(float mixPercent);

    /**
     * @brief Blend dry and wet signals
     * @param dry Uncompressed (dry) sample
     * @param wet Compressed (wet) sample
     * @return Blended output sample
     */
    float process(float dry, float wet) const;

private:
    float m_wetGain = 1.0f;
    float m_dryGain = 0.0f;
};

/**
 * @class AutoMakeupGain
 * @brief Automatic makeup gain computation
 * 
 * Estimates and applies makeup gain to compensate for
 * gain reduction, maintaining perceived loudness.
 * 
 * RT-Safe: Yes (no allocations)
 * Complexity: O(1)
 */
class AutoMakeupGain
{
public:
    AutoMakeupGain() = default;
    ~AutoMakeupGain() = default;

    /**
     * @brief Prepare for processing
     * @param sampleRate Sample rate in Hz
     */
    void prepare(double sampleRate);

    /**
     * @brief Enable/disable auto-makeup
     * @param enabled True to enable
     */
    void setEnabled(bool enabled);

    /**
     * @brief Estimate makeup gain from compressor settings
     * @param threshold Threshold in dB
     * @param ratio Compression ratio
     * @return Estimated makeup gain in dB
     */
    float estimateMakeupGain(float threshold, float ratio) const;

    /**
     * @brief Update adaptive makeup based on actual GR
     * @param currentGR Current gain reduction in dB (negative)
     */
    void updateAdaptive(float currentGR);

    /**
     * @brief Get current makeup gain
     * @return Makeup gain in dB
     */
    float getMakeupGain() const;

private:
    bool m_enabled = true;
    float m_adaptiveGain = 0.0f;
    float m_smoothingCoeff = 0.001f;  // Slow adaptation
    double m_sampleRate = 48000.0;
};

/**
 * @class ProgramDependentRelease
 * @brief Adaptive release time based on input characteristics
 * 
 * Automatically adjusts release time based on signal characteristics
 * to prevent pumping on transient material while remaining smooth
 * on sustained material.
 * 
 * RT-Safe: Yes (no allocations)
 * Complexity: O(1)
 */
class ProgramDependentRelease
{
public:
    ProgramDependentRelease() = default;
    ~ProgramDependentRelease() = default;

    /**
     * @brief Prepare for processing
     * @param sampleRate Sample rate in Hz
     */
    void prepare(double sampleRate);

    /**
     * @brief Set base release time
     * @param releaseMs Base release time in ms
     */
    void setBaseRelease(float releaseMs);

    /**
     * @brief Set fast release time (for transients)
     * @param releaseMs Fast release in ms
     */
    void setFastRelease(float releaseMs);

    /**
     * @brief Compute adaptive release time
     * @param inputLevel Current input level in dB
     * @param grAmount Current GR amount in dB (magnitude)
     * @return Adapted release time in ms
     */
    float computeReleaseTime(float inputLevel, float grAmount);

private:
    double m_sampleRate = 48000.0;
    float m_baseRelease = 250.0f;
    float m_fastRelease = 50.0f;
    float m_prevLevel = -120.0f;
    float m_transientThreshold = 6.0f;  // dB change for transient detection
};

} // namespace DSP
} // namespace EzSqueeze


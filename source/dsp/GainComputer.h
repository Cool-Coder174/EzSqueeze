/**
 * @file GainComputer.h
 * @brief Gain reduction computation for dynamic range compression
 * 
 * Implements the static compression curve with configurable
 * threshold, ratio, and knee characteristics.
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
 * @brief Knee transition mode
 */
enum class KneeMode
{
    Hard,    ///< Sharp transition (0.1 dB)
    Medium,  ///< Moderate transition (3 dB)
    Soft     ///< Smooth transition (6 dB)
};

/**
 * @class GainComputer
 * @brief Computes gain reduction from input level
 * 
 * The GainComputer implements the static compression curve
 * that determines how much gain reduction to apply based on
 * the input signal level, threshold, ratio, and knee settings.
 * 
 * RT-Safe: Yes (no allocations, pure computation)
 * Complexity: O(1) per sample
 */
class GainComputer
{
public:
    GainComputer() = default;
    ~GainComputer() = default;

    /**
     * @brief Set compression threshold
     * @param thresholdDb Threshold in dBFS (-60 to 0)
     */
    void setThreshold(float thresholdDb);

    /**
     * @brief Set compression ratio
     * @param ratio Compression ratio (1.0 to 32.0, where 1.0 = no compression)
     */
    void setRatio(float ratio);

    /**
     * @brief Set knee mode
     * @param mode Hard, Medium, or Soft knee
     */
    void setKneeMode(KneeMode mode);

    /**
     * @brief Compute gain reduction for given input level
     * @param inputLevelDb Input level in dBFS
     * @return Gain reduction in dB (always negative or zero)
     */
    float computeGainReduction(float inputLevelDb) const;

    /**
     * @brief Get current threshold
     * @return Threshold in dB
     */
    float getThreshold() const { return m_threshold; }

    /**
     * @brief Get current ratio
     * @return Compression ratio
     */
    float getRatio() const { return m_ratio; }

    /**
     * @brief Get current knee mode
     * @return Knee mode enumeration
     */
    KneeMode getKneeMode() const { return m_kneeMode; }

private:
    float m_threshold = -20.0f;
    float m_ratio = 4.0f;
    KneeMode m_kneeMode = KneeMode::Medium;
    float m_kneeWidth = 3.0f;  // in dB

    /**
     * @brief Update knee width based on current knee mode
     */
    void updateKneeWidth();

    /**
     * @brief Compute hard knee gain reduction
     * @param overshoot Amount above threshold in dB
     * @return Gain reduction in dB
     */
    inline float computeHardKnee(float overshoot) const
    {
        return overshoot * (1.0f - 1.0f / m_ratio);
    }

    /**
     * @brief Compute soft knee gain reduction
     * @param inputLevelDb Input level in dB
     * @return Gain reduction in dB
     */
    float computeSoftKnee(float inputLevelDb) const;
};

} // namespace DSP
} // namespace EzSqueeze

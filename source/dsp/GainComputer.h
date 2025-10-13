#pragma once

#include <cmath>
#include <algorithm>

namespace EzSqueeze::DSP {

/**
 * @brief Gain computer for calculating compression gain reduction
 * 
 * Implements hard, medium, and soft knee compression curves with
 * mathematical accuracy and real-time safety.
 */
class GainComputer
{
public:
    enum class KneeType
    {
        Hard,   ///< 0 dB transition (instantaneous)
        Medium, ///< 3 dB transition (smooth)
        Soft    ///< 6 dB transition (very smooth)
    };

    /**
     * @brief Constructor
     * @param threshold Compression threshold in dBFS
     * @param ratio Compression ratio (1.0 = no compression, 4.0 = 4:1)
     * @param knee Knee type for transition smoothness
     */
    GainComputer(float threshold = -18.0f, float ratio = 4.0f, KneeType knee = KneeType::Medium);

    /**
     * @brief Set compression parameters
     * @param threshold New threshold in dBFS
     * @param ratio New compression ratio
     * @param knee New knee type
     */
    void setParameters(float threshold, float ratio, KneeType knee);

    /**
     * @brief Calculate gain reduction for a given input level
     * @param inputLevel Input level in dBFS
     * @return Gain reduction in dB (negative value)
     */
    float calculateGainReduction(float inputLevel) const;

    /**
     * @brief Process a block of input levels
     * @param inputLevels Input levels in dBFS
     * @param gainReduction Output gain reduction in dB
     * @param numSamples Number of samples to process
     */
    void processBlock(const float* inputLevels, float* gainReduction, int numSamples) const;

    /**
     * @brief Get current threshold
     * @return Threshold in dBFS
     */
    float getThreshold() const { return threshold_; }

    /**
     * @brief Get current ratio
     * @return Compression ratio
     */
    float getRatio() const { return ratio_; }

    /**
     * @brief Get current knee type
     * @return Knee type
     */
    KneeType getKneeType() const { return knee_; }

    /**
     * @brief Get knee width in dB
     * @return Knee width
     */
    float getKneeWidth() const;

    /**
     * @brief Check if input level is within knee region
     * @param inputLevel Input level in dBFS
     * @return True if within knee region
     */
    bool isInKneeRegion(float inputLevel) const;

private:
    float threshold_;
    float ratio_;
    KneeType knee_;
    float kneeWidth_;
    float kneeStart_;
    float kneeEnd_;

    /**
     * @brief Update knee parameters based on current settings
     */
    void updateKneeParameters();

    /**
     * @brief Calculate gain reduction for hard knee
     * @param inputLevel Input level in dBFS
     * @return Gain reduction in dB
     */
    float calculateHardKnee(float inputLevel) const;

    /**
     * @brief Calculate gain reduction for soft knee using parabolic interpolation
     * @param inputLevel Input level in dBFS
     * @return Gain reduction in dB
     */
    float calculateSoftKnee(float inputLevel) const;

    /**
     * @brief Linear interpolation between two values
     * @param a Start value
     * @param b End value
     * @param t Interpolation factor (0.0 to 1.0)
     * @return Interpolated value
     */
    float lerp(float a, float b, float t) const;

    /**
     * @brief Clamp value between min and max
     * @param value Value to clamp
     * @param min Minimum value
     * @param max Maximum value
     * @return Clamped value
     */
    float clamp(float value, float min, float max) const;
};

} // namespace EzSqueeze::DSP
/**
 * @file Saturation.h
 * @brief Saturation/distortion with multiple curve types
 * 
 * Provides various saturation algorithms for harmonic coloration:
 * - Soft clip (tanh-based)
 * - Even harmonics (tube-like)
 * - Odd harmonics (transistor-like)
 * - Tape saturation (asymmetric)
 * 
 * @author Isaac Hernandez
 * @date October 2025
 */

#pragma once

#include <cmath>
#include <algorithm>

namespace EzSqueeze {
namespace Modules {

/**
 * @brief Saturation curve types
 */
enum class SaturationCurve
{
    Off,          ///< Bypass saturation
    SoftClip,     ///< Tanh-based soft clipping
    EvenHarmonics,///< Tube-style even harmonics
    OddHarmonics, ///< Transistor-style odd harmonics
    Tape          ///< Tape-style asymmetric saturation
};

/**
 * @class Saturation
 * @brief Flexible saturation processor
 * 
 * Provides multiple saturation curves for adding harmonic content
 * and coloration. All curves are designed to be gentle and musical.
 * 
 * **Important**: Saturation should be used with oversampling to
 * prevent aliasing artifacts. Without oversampling, use drive < 0.3.
 * 
 * RT-Safe: Yes (no allocations)
 * Complexity: O(1) per sample
 */
class Saturation
{
public:
    Saturation() = default;
    ~Saturation() = default;

    /**
     * @brief Set saturation curve type
     * @param curve Curve enumeration
     */
    void setCurve(SaturationCurve curve);

    /**
     * @brief Set drive amount
     * @param drive Drive amount (0.0 = clean, 1.0 = heavy)
     */
    void setDrive(float drive);

    /**
     * @brief Set tone trim
     * @param tone Tone control (-1.0 = dark, 0.0 = neutral, +1.0 = bright)
     */
    void setTone(float tone);

    /**
     * @brief Process single sample
     * @param input Input sample
     * @return Saturated output sample
     */
    float processSample(float input);

private:
    SaturationCurve m_curve = SaturationCurve::Off;
    float m_drive = 0.0f;
    float m_tone = 0.0f;
    float m_driveGain = 1.0f;
    float m_compensationGain = 1.0f;

    /**
     * @brief Update internal gain values
     */
    void updateGains();

    /**
     * @brief Soft clip using tanh
     * @param x Input value
     * @return Clipped output
     */
    inline float softClip(float x) const
    {
        return std::tanh(x * m_driveGain) / std::tanh(m_driveGain);
    }

    /**
     * @brief Even harmonic distortion
     * @param x Input value
     * @return Output with even harmonics
     */
    inline float evenHarmonicsSat(float x) const
    {
        const float driven = x * m_driveGain;
        return driven + 0.1f * (driven * driven);
    }

    /**
     * @brief Odd harmonic distortion
     * @param x Input value
     * @return Output with odd harmonics
     */
    inline float oddHarmonicsSat(float x) const
    {
        const float driven = x * m_driveGain;
        return driven + 0.15f * (driven * driven * driven);
    }

    /**
     * @brief Tape-style asymmetric saturation
     * @param x Input value
     * @return Tape-saturated output
     */
    float tapeSaturation(float x) const;

    /**
     * @brief Apply tone control
     * @param x Input value
     * @return Tone-adjusted output
     */
    inline float applyTone(float x) const
    {
        // Simple tilt: boost highs or lows based on tone
        // In a full implementation, this would use filters
        return x * (1.0f + m_tone * 0.2f);
    }
};

} // namespace Modules
} // namespace EzSqueeze


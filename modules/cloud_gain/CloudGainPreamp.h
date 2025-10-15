/**
 * @file CloudGainPreamp.h
 * @brief Clean preamp with character modeling
 * 
 * Provides +0 to +30dB of clean gain with optional impedance
 * character modeling (Silicon/Tube/Transformer) for subtle
 * harmonic coloration.
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
 * @brief Impedance character modes
 */
enum class ImpedanceMode
{
    Silicon,      ///< Clean, minimal harmonics
    Tube,         ///< Even harmonics (2nd, 4th)
    Transformer   ///< Even + odd harmonics, LF bump
};

/**
 * @class CloudGainPreamp
 * @brief Clean preamp with impedance character
 * 
 * The CloudGainPreamp provides transparent gain with optional
 * character modeling. It targets a noise floor < -110 dBFS
 * and uses subtle waveshaping for harmonic coloration.
 * 
 * RT-Safe: Yes (no allocations)
 * Complexity: O(1) per sample
 */
class CloudGainPreamp
{
public:
    CloudGainPreamp() = default;
    ~CloudGainPreamp() = default;

    /**
     * @brief Prepare preamp for processing
     * @param sampleRate Sample rate in Hz
     */
    void prepare(double sampleRate);

    /**
     * @brief Set preamp gain
     * @param gainDb Gain in dB (0 to +30)
     */
    void setGain(float gainDb);

    /**
     * @brief Set impedance character mode
     * @param mode Silicon, Tube, or Transformer
     */
    void setImpedanceMode(ImpedanceMode mode);

    /**
     * @brief Set character amount
     * @param amount Character intensity (0.0 = clean, 1.0 = full)
     */
    void setCharacterAmount(float amount);

    /**
     * @brief Process single sample
     * @param input Input sample
     * @return Amplified output sample with character
     */
    float processSample(float input);

    /**
     * @brief Reset preamp state
     */
    void reset();

private:
    double m_sampleRate = 48000.0;
    float m_gainLinear = 1.0f;
    ImpedanceMode m_mode = ImpedanceMode::Silicon;
    float m_characterAmount = 0.5f;

    // LF bump filter for transformer mode
    float m_lfBumpZ1 = 0.0f;
    float m_lfBumpCoeff = 0.0f;

    /**
     * @brief Apply impedance character to sample
     * @param sample Input sample (post-gain)
     * @return Sample with character applied
     */
    float applyCharacter(float sample);

    /**
     * @brief Soft saturation function
     * @param x Input value
     * @return Saturated output
     */
    inline float softSaturate(float x) const
    {
        return std::tanh(x);
    }

    /**
     * @brief Apply even harmonics (tube-like)
     * @param x Input value
     * @return Output with even harmonics
     */
    inline float evenHarmonics(float x) const
    {
        // Add subtle 2nd harmonic
        return x + m_characterAmount * 0.05f * (x * x);
    }

    /**
     * @brief Apply transformer characteristics
     * @param x Input value
     * @return Output with transformer coloration
     */
    float transformerColor(float x);

    /**
     * @brief Update LF bump filter coefficient
     */
    void updateLFBumpFilter();
};

} // namespace Modules
} // namespace EzSqueeze


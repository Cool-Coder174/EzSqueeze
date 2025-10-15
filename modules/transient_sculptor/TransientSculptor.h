/**
 * @file TransientSculptor.h
 * @brief Transient shaping with Snap/Body/De-Snap controls
 * 
 * Provides intuitive transient control by modifying attack/release
 * parameters and applying fast limiting to transients.
 * 
 * @author Isaac Hernandez
 * @date October 2025
 */

#pragma once

#include "../../source/dsp/EnvelopeFollower.h"
#include <cmath>
#include <algorithm>

namespace EzSqueeze {
namespace Modules {

/**
 * @class TransientSculptor
 * @brief Intuitive transient shaping processor
 * 
 * The TransientSculptor provides user-friendly transient control:
 * - **Snap**: Bias attack shorter (emphasize transients)
 * - **Body**: Bias release longer (emphasize sustain)
 * - **De-Snap**: Fast limiting on transients (soften spikes)
 * 
 * Maps internally to attack/release modifications and fast peak limiting.
 * 
 * RT-Safe: Yes (no allocations)
 * Complexity: O(1) per sample
 */
class TransientSculptor
{
public:
    TransientSculptor() = default;
    ~TransientSculptor() = default;

    /**
     * @brief Prepare transient sculptor
     * @param sampleRate Sample rate in Hz
     */
    void prepare(double sampleRate);

    /**
     * @brief Set snap amount (transient emphasis)
     * @param snap Snap amount (0.0 = normal, 1.0 = max emphasis)
     */
    void setSnap(float snap);

    /**
     * @brief Set body amount (sustain emphasis)
     * @param body Body amount (0.0 = normal, 1.0 = max emphasis)
     */
    void setBody(float body);

    /**
     * @brief Set de-snap amount (transient softening)
     * @param deSnap De-snap amount (0.0 = off, 1.0 = max softening)
     */
    void setDeSnap(float deSnap);

    /**
     * @brief Get modified attack time
     * @param baseAttackMs Base attack time in ms
     * @return Modified attack time in ms
     */
    float getModifiedAttack(float baseAttackMs) const;

    /**
     * @brief Get modified release time
     * @param baseReleaseMs Base release time in ms
     * @return Modified release time in ms
     */
    float getModifiedRelease(float baseReleaseMs) const;

    /**
     * @brief Apply transient limiting (de-snap)
     * @param input Input sample
     * @return Limited output sample
     */
    float applyDeSnap(float input);

    /**
     * @brief Reset sculptor state
     */
    void reset();

private:
    double m_sampleRate = 48000.0;

    float m_snap = 0.0f;     // Attack bias: 0.5-2.0
    float m_body = 0.0f;     // Release bias: 0.5-2.0
    float m_deSnap = 0.0f;   // Transient limiting amount

    // Fast transient limiter
    float m_transientPeak = 0.0f;
    float m_fastAttackCoeff = 0.0f;
    float m_fastReleaseCoeff = 0.0f;

    /**
     * @brief Update fast limiter coefficients
     */
    void updateLimiterCoeffs();

    /**
     * @brief Map 0-1 control to 0.5-2.0 bias
     * @param control Control value (0-1)
     * @return Bias multiplier (0.5-2.0)
     */
    inline float controlToBias(float control) const
    {
        // Map 0.0 → 1.0 (neutral)
        // Map 0.5 → 1.0 (neutral)
        // Map 1.0 → 2.0 (max)
        return 1.0f + control;  // 1.0 to 2.0 for emphasis
    }

    /**
     * @brief Convert time to coefficient
     * @param timeMs Time in milliseconds
     * @return Smoothing coefficient
     */
    float timeToCoeff(float timeMs) const;
};

} // namespace Modules
} // namespace EzSqueeze


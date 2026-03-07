#pragma once

#include "../source/dsp/EnvelopeFollower.h"

#include <cmath>
#include <algorithm>

namespace ezsqueeze
{

/**
 * @brief Transient shaper with Snap / Body / De-Snap controls.
 *
 * Modifies the effective attack and release times fed to the
 * compressor and provides a fast peak limiter (De-Snap):
 *
 *  - **Snap (0–1)**: shortens the compressor attack time,
 *    allowing the compressor to grab transients more aggressively
 *    and sculpt the initial hit.
 *  - **Body (0–1)**: lengthens the compressor release time,
 *    emphasising sustain and body of the signal.
 *  - **De-Snap (0–1)**: engages a fast limiter (0.1 ms attack)
 *    that tames transient peaks before they reach the compressor.
 */
class TransientSculptor
{
public:
    /** @brief Prepare the module for a given sample rate. */
    void setSampleRate(double sr)
    {
        sampleRate_ = sr;
        recalcDeSnapCoeffs();
    }

    /**
     * @brief Set the snap (transient emphasis) amount.
     * @param v  0.0 = off, 1.0 = maximum snap.
     */
    void setSnap(float v) { snap_ = std::clamp(v, 0.0f, 1.0f); }

    /**
     * @brief Set the body (sustain emphasis) amount.
     * @param v  0.0 = off, 1.0 = maximum body.
     */
    void setBody(float v) { body_ = std::clamp(v, 0.0f, 1.0f); }

    /**
     * @brief Set the de-snap (transient limiter) amount.
     * @param v  0.0 = off, 1.0 = maximum limiting.
     */
    void setDeSnap(float v)
    {
        deSnap_ = std::clamp(v, 0.0f, 1.0f);
        recalcDeSnapCoeffs();
    }

    /**
     * @brief Return a modified attack time based on the snap setting.
     *
     * At snap = 0 the base attack is returned unchanged.
     * At snap = 1 the attack is reduced to 5 % of its base value.
     *
     * @param baseAttackMs  The compressor's base attack time in ms.
     * @return Modified attack time in ms.
     */
    float getModifiedAttack(float baseAttackMs) const
    {
        float factor = 1.0f - snap_ * 0.95f;
        return std::max(baseAttackMs * factor, MIN_ATTACK_MS);
    }

    /**
     * @brief Return a modified release time based on the body setting.
     *
     * At body = 0 the base release is returned unchanged.
     * At body = 1 the release is scaled to 5× its base value.
     *
     * @param baseReleaseMs  The compressor's base release time in ms.
     * @return Modified release time in ms.
     */
    float getModifiedRelease(float baseReleaseMs) const
    {
        float factor = 1.0f + body_ * 4.0f;
        return baseReleaseMs * factor;
    }

    /**
     * @brief Apply the De-Snap fast limiter to a stereo sample pair.
     *
     * Uses a 0.1 ms attack envelope to catch and attenuate transient
     * peaks. The threshold scales with the de-snap amount (at 1.0
     * the threshold is −3 dBFS).
     *
     * @param[in,out] left   Left channel sample.
     * @param[in,out] right  Right channel sample.
     */
    void processDeSnap(float& left, float& right)
    {
        if (deSnap_ < 0.001f)
            return;

        float threshDB = -3.0f * deSnap_;
        float threshold = std::pow(10.0f, threshDB / 20.0f);

        float peak = std::max(std::fabs(left), std::fabs(right));

        if (peak > threshold)
        {
            float target = threshold / peak;
            limiterEnv_ += deSnapAttackCoeff_ * (target - limiterEnv_);
        }
        else
        {
            limiterEnv_ += deSnapReleaseCoeff_ * (1.0f - limiterEnv_);
        }

        limiterEnv_ = std::clamp(limiterEnv_, 0.0f, 1.0f);
        left  *= limiterEnv_;
        right *= limiterEnv_;
    }

    /** @brief Reset all internal state. */
    void reset()
    {
        limiterEnv_ = 1.0f;
    }

private:
    static constexpr float MIN_ATTACK_MS = 0.01f;
    static constexpr float DE_SNAP_ATTACK_MS  = 0.1f;
    static constexpr float DE_SNAP_RELEASE_MS = 50.0f;

    void recalcDeSnapCoeffs()
    {
        if (sampleRate_ <= 0.0)
            return;
        deSnapAttackCoeff_  = 1.0f - std::exp(
            -1.0f / (DE_SNAP_ATTACK_MS * 0.001f * static_cast<float>(sampleRate_)));
        deSnapReleaseCoeff_ = 1.0f - std::exp(
            -1.0f / (DE_SNAP_RELEASE_MS * 0.001f * static_cast<float>(sampleRate_)));
    }

    double sampleRate_ = 44100.0;
    float snap_ = 0.0f;
    float body_ = 0.0f;
    float deSnap_ = 0.0f;
    float limiterEnv_ = 1.0f;
    float deSnapAttackCoeff_ = 0.1f;
    float deSnapReleaseCoeff_ = 0.001f;
};

} // namespace ezsqueeze

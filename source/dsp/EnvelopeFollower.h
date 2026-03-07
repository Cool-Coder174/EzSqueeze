#pragma once

#include <cmath>
#include <algorithm>

namespace ezsqueeze
{

/**
 * @brief Attack/release envelope follower with ballistic smoothing.
 *
 * Smooths a control signal (e.g. gain reduction) using separate
 * attack and release time constants. Uses 1-pole IIR filtering
 * where coefficients are derived from:
 *   alpha = 1 - exp(-1 / (time_ms * 0.001 * sampleRate))
 */
class EnvelopeFollower
{
public:
    /** @brief Set the sample rate for coefficient calculations. */
    void setSampleRate(double sr)
    {
        sampleRate_ = sr;
        recalcAttack();
        recalcRelease();
    }

    /**
     * @brief Set the attack time.
     * @param ms  Attack time in milliseconds (>= 0.01).
     */
    void setAttack(float ms)
    {
        attackMs_ = std::max(ms, 0.01f);
        recalcAttack();
    }

    /**
     * @brief Set the release time.
     * @param ms  Release time in milliseconds (>= 0.01).
     */
    void setRelease(float ms)
    {
        releaseMs_ = std::max(ms, 0.01f);
        recalcRelease();
    }

    /**
     * @brief Process a single input value through the envelope.
     * @param input  Input value (e.g. detected level or gain reduction).
     * @return Smoothed output value.
     */
    float process(float input)
    {
        float coeff = (input > state_) ? attackCoeff_ : releaseCoeff_;
        state_ += coeff * (input - state_);
        return state_;
    }

    /** @brief Get the current envelope state. */
    float getState() const { return state_; }

    /** @brief Reset the envelope state to zero. */
    void reset()
    {
        state_ = 0.0f;
    }

private:
    void recalcAttack()
    {
        if (sampleRate_ <= 0.0)
            return;
        attackCoeff_ = 1.0f - std::exp(-1.0f / (attackMs_ * 0.001f * static_cast<float>(sampleRate_)));
    }

    void recalcRelease()
    {
        if (sampleRate_ <= 0.0)
            return;
        releaseCoeff_ = 1.0f - std::exp(-1.0f / (releaseMs_ * 0.001f * static_cast<float>(sampleRate_)));
    }

    double sampleRate_ = 44100.0;
    float attackMs_ = 1.0f;
    float releaseMs_ = 100.0f;
    float attackCoeff_ = 0.1f;
    float releaseCoeff_ = 0.001f;
    float state_ = 0.0f;
};

} // namespace ezsqueeze

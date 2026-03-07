#pragma once

#include <cmath>
#include <algorithm>

namespace ezsqueeze
{

/**
 * @brief Compression gain-curve calculator with selectable knee modes.
 *
 * Computes gain reduction in dB for a given input level in dB, based on
 * threshold, ratio, and knee settings. Supports hard, medium (3 dB),
 * and soft (6 dB) knee widths with parabolic interpolation.
 */
class GainComputer
{
public:
    /** Knee mode with associated width in dB. */
    enum class KneeMode
    {
        Hard,   ///< 0 dB knee width — sharp breakpoint
        Medium, ///< 3 dB knee width
        Soft    ///< 6 dB knee width
    };

    /**
     * @brief Set the compression threshold.
     * @param dB  Threshold in dBFS (typically negative).
     */
    void setThreshold(float dB)
    {
        threshold_ = dB;
    }

    /**
     * @brief Set the compression ratio.
     * @param r  Ratio (e.g. 4.0 for 4:1). Clamped to >= 1.0.
     */
    void setRatio(float r)
    {
        ratio_ = std::max(r, 1.0f);
    }

    /**
     * @brief Set the knee mode.
     * @param mode  Hard, Medium, or Soft knee.
     */
    void setKnee(KneeMode mode)
    {
        kneeMode_ = mode;
        switch (mode)
        {
            case KneeMode::Hard:   kneeWidth_ = 0.0f; break;
            case KneeMode::Medium: kneeWidth_ = 3.0f; break;
            case KneeMode::Soft:   kneeWidth_ = 6.0f; break;
        }
    }

    /**
     * @brief Compute gain reduction for a given input level.
     * @param inputDB  Input signal level in dB.
     * @return Gain reduction in dB (positive value = attenuation).
     */
    float compute(float inputDB) const
    {
        float overshoot = inputDB - threshold_;

        if (kneeWidth_ < 0.001f)
        {
            if (overshoot <= 0.0f)
                return 0.0f;
            return overshoot * (1.0f - 1.0f / ratio_);
        }

        float halfKnee = kneeWidth_ * 0.5f;

        if (overshoot <= -halfKnee)
        {
            return 0.0f;
        }
        else if (overshoot >= halfKnee)
        {
            return overshoot * (1.0f - 1.0f / ratio_);
        }
        else
        {
            float t = overshoot + halfKnee;
            return (t * t) * (1.0f - 1.0f / ratio_) / (2.0f * kneeWidth_);
        }
    }

    /** @brief Get the current threshold in dB. */
    float getThreshold() const { return threshold_; }

    /** @brief Get the current ratio. */
    float getRatio() const { return ratio_; }

private:
    float threshold_ = -20.0f;
    float ratio_ = 4.0f;
    KneeMode kneeMode_ = KneeMode::Hard;
    float kneeWidth_ = 0.0f;
};

} // namespace ezsqueeze

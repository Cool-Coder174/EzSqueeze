#pragma once

#include <cmath>
#include <algorithm>

namespace ezsqueeze
{

/**
 * @brief Automatic makeup gain compensation.
 *
 * Provides two strategies:
 *  - **Static**: estimates makeup gain from threshold + ratio assuming
 *    a typical 10 dB overshoot (70 % compensation).
 *  - **Adaptive**: tracks a running average of actual gain reduction
 *    and compensates at 80 %.
 */
class AutoMakeup
{
public:
    /**
     * @brief Estimate makeup gain from threshold and ratio (static mode).
     *
     * Assumes the signal overshoots the threshold by ~10 dB on average
     * and applies a 70 % compensation factor.
     *
     * @param threshold  Compressor threshold in dB.
     * @param ratio      Compression ratio (e.g. 4.0 for 4:1).
     * @return Estimated makeup gain in dB (positive = boost).
     */
    float computeStatic(float threshold, float ratio) const
    {
        float safeRatio = std::max(ratio, 1.0f);
        float grAtOvershoot = ASSUMED_OVERSHOOT_DB * (1.0f - 1.0f / safeRatio);
        return grAtOvershoot * STATIC_COMPENSATION;
    }

    /**
     * @brief Feed current gain reduction to the adaptive tracker.
     *
     * Call once per sample (or per block with the block's average GR).
     * Uses a leaky integrator to track mean GR, then scales by 80 %.
     *
     * @param currentGR  Current gain reduction in dB (positive value).
     */
    void updateAdaptive(float currentGR)
    {
        avgGR_ += SMOOTH_COEFF * (currentGR - avgGR_);
    }

    /**
     * @brief Return the current adaptive makeup gain in dB.
     * @return Makeup gain (positive = boost).
     */
    float getGainDB() const
    {
        return avgGR_ * ADAPTIVE_COMPENSATION;
    }

    /** @brief Reset the adaptive running average. */
    void reset()
    {
        avgGR_ = 0.0f;
    }

private:
    static constexpr float ASSUMED_OVERSHOOT_DB = 10.0f;
    static constexpr float STATIC_COMPENSATION  = 0.70f;
    static constexpr float ADAPTIVE_COMPENSATION = 0.80f;
    static constexpr float SMOOTH_COEFF = 0.0001f;

    float avgGR_ = 0.0f;
};

} // namespace ezsqueeze

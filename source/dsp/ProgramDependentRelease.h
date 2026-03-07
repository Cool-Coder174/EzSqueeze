#pragma once

#include <cmath>
#include <algorithm>

namespace ezsqueeze
{

/**
 * @brief Adaptive release time based on signal transient characteristics.
 *
 * Selects between a fast and slow release time depending on whether
 * the input signal contains a transient (large level jump) while
 * significant gain reduction is active (> 3 dB). Provides a
 * smooth blend between the two extremes.
 */
class ProgramDependentRelease
{
public:
    /** @brief Set the sample rate for internal smoothing. */
    void setSampleRate(double sr)
    {
        sampleRate_ = sr;
        recalcSmoothing();
    }

    /**
     * @brief Set the fast release time (used on transients).
     * @param ms  Time in milliseconds.
     */
    void setFastRelease(float ms)
    {
        fastReleaseMs_ = std::max(ms, 1.0f);
    }

    /**
     * @brief Set the slow release time (used for sustained signals).
     * @param ms  Time in milliseconds.
     */
    void setSlowRelease(float ms)
    {
        slowReleaseMs_ = std::max(ms, 1.0f);
    }

    /**
     * @brief Compute the adaptive release time for the current sample.
     *
     * A transient is detected when the absolute level change exceeds
     * the transient threshold AND gain reduction is greater than 3 dB.
     * In that case the fast release is selected; otherwise the output
     * smoothly blends toward the slow release.
     *
     * @param inputLevel  Current detected input level (linear or dB).
     * @param prevLevel   Previous detected input level (same domain).
     * @param grAmount    Current gain reduction in dB (positive).
     * @return Effective release time in milliseconds.
     */
    float computeRelease(float inputLevel, float prevLevel, float grAmount)
    {
        float levelDelta = std::fabs(inputLevel - prevLevel);
        bool isTransient = (levelDelta > TRANSIENT_THRESHOLD) && (grAmount > GR_THRESHOLD_DB);

        float target = isTransient ? fastReleaseMs_ : slowReleaseMs_;

        currentRelease_ += smoothCoeff_ * (target - currentRelease_);

        return currentRelease_;
    }

    /** @brief Get the last computed release time in ms. */
    float getCurrentRelease() const { return currentRelease_; }

    /** @brief Reset the smoothing state. */
    void reset()
    {
        currentRelease_ = slowReleaseMs_;
    }

private:
    static constexpr float TRANSIENT_THRESHOLD = 0.1f;
    static constexpr float GR_THRESHOLD_DB = 3.0f;
    static constexpr float SMOOTH_TIME_MS = 50.0f;

    void recalcSmoothing()
    {
        if (sampleRate_ <= 0.0)
            return;
        smoothCoeff_ = 1.0f - std::exp(-1.0f / (SMOOTH_TIME_MS * 0.001f * static_cast<float>(sampleRate_)));
    }

    double sampleRate_ = 44100.0;
    float fastReleaseMs_ = 50.0f;
    float slowReleaseMs_ = 500.0f;
    float currentRelease_ = 500.0f;
    float smoothCoeff_ = 0.001f;
};

} // namespace ezsqueeze

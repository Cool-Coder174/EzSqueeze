#pragma once

#include "Saturation.h"
#include <cmath>
#include <algorithm>

namespace ezsqueeze
{

/**
 * @brief Blends from perfectly clean (0) to vintage warmth (1).
 *
 * Progressively applies:
 *  - Soft-clip saturation (increasing drive)
 *  - Even-harmonic distortion
 *  - A subtle 1-pole low-pass filter (decreasing cutoff)
 *
 * At 0 the signal is untouched. At 0.5 gentle warmth is added.
 * At 1.0 the full vintage character is present.
 */
class VibeWheel
{
public:
    /** @brief Set the processing sample rate. */
    void setSampleRate(double sr)
    {
        sampleRate_ = sr;
        recalcLPF();
    }

    /**
     * @brief Set the vibe amount.
     * @param a  0.0 = clean, 1.0 = full vintage warmth.
     */
    void setAmount(float a)
    {
        amount_ = std::clamp(a, 0.0f, 1.0f);
        recalcLPF();
    }

    /**
     * @brief Apply the vibe processing to a single sample in-place.
     * @param[in,out] sample  Audio sample to process.
     */
    void process(float& sample)
    {
        if (amount_ < 0.001f)
            return;

        float drive = 1.0f + amount_ * MAX_DRIVE;
        float saturated = Saturation::softClip(sample, drive);
        sample += amount_ * (saturated - sample);

        sample = Saturation::evenHarmonics(sample, amount_ * MAX_EVEN_HARMONICS);

        lpfState_ += lpfCoeff_ * (sample - lpfState_);
        sample += amount_ * (lpfState_ - sample);
    }

    /** @brief Reset the low-pass filter state. */
    void reset()
    {
        lpfState_ = 0.0f;
    }

private:
    static constexpr float PI = 3.14159265358979323846f;
    static constexpr float MAX_DRIVE = 1.5f;
    static constexpr float MAX_EVEN_HARMONICS = 0.15f;
    static constexpr float LPF_HI_HZ = 20000.0f;
    static constexpr float LPF_LO_HZ = 8000.0f;

    void recalcLPF()
    {
        if (sampleRate_ <= 0.0)
            return;
        float cutoff = LPF_HI_HZ - amount_ * (LPF_HI_HZ - LPF_LO_HZ);
        lpfCoeff_ = 1.0f - std::exp(-2.0f * PI * cutoff / static_cast<float>(sampleRate_));
    }

    double sampleRate_ = 44100.0;
    float amount_ = 0.0f;
    float lpfState_ = 0.0f;
    float lpfCoeff_ = 1.0f;
};

} // namespace ezsqueeze

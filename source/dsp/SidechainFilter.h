#pragma once

#include <cmath>
#include <algorithm>

namespace ezsqueeze
{

/**
 * @brief HPF + LPF biquad chain for sidechain filtering.
 *
 * Applies a high-pass filter (20–400 Hz) followed by a low-pass
 * filter (4000–16000 Hz) to the sidechain signal. Both filters
 * use Butterworth alignment (Q = 0.7071) implemented with
 * transposed direct-form II biquads.
 */
class SidechainFilter
{
public:
    /** @brief Set the sample rate and recalculate all coefficients. */
    void setSampleRate(double sr)
    {
        sampleRate_ = sr;
        recalcHPF();
        recalcLPF();
    }

    /**
     * @brief Set the high-pass filter cutoff frequency.
     * @param freq  Cutoff in Hz, clamped to 20–400 Hz.
     */
    void setHPF(float freq)
    {
        hpfFreq_ = std::clamp(freq, HPF_MIN_HZ, HPF_MAX_HZ);
        recalcHPF();
    }

    /**
     * @brief Set the low-pass filter cutoff frequency.
     * @param freq  Cutoff in Hz, clamped to 4000–16000 Hz.
     */
    void setLPF(float freq)
    {
        lpfFreq_ = std::clamp(freq, LPF_MIN_HZ, LPF_MAX_HZ);
        recalcLPF();
    }

    /**
     * @brief Process a single sample through HPF then LPF.
     * @param input  Input sample.
     * @return Filtered output.
     */
    float process(float input)
    {
        float out = hpf_.process(input);
        return lpf_.process(out);
    }

    /** @brief Reset all filter state to zero. */
    void reset()
    {
        hpf_.reset();
        lpf_.reset();
    }

private:
    static constexpr float PI = 3.14159265358979323846f;
    static constexpr float BUTTERWORTH_Q = 0.7071067811865476f;
    static constexpr float HPF_MIN_HZ = 20.0f;
    static constexpr float HPF_MAX_HZ = 400.0f;
    static constexpr float LPF_MIN_HZ = 4000.0f;
    static constexpr float LPF_MAX_HZ = 16000.0f;

    struct Biquad
    {
        float b0 = 1.0f, b1 = 0.0f, b2 = 0.0f;
        float a1 = 0.0f, a2 = 0.0f;
        float z1 = 0.0f, z2 = 0.0f;

        float process(float x)
        {
            float y = b0 * x + z1;
            z1 = b1 * x - a1 * y + z2;
            z2 = b2 * x - a2 * y;
            return y;
        }

        void reset()
        {
            z1 = 0.0f;
            z2 = 0.0f;
        }
    };

    void recalcHPF()
    {
        if (sampleRate_ <= 0.0)
            return;

        float w0 = 2.0f * PI * hpfFreq_ / static_cast<float>(sampleRate_);
        float cosW0 = std::cos(w0);
        float sinW0 = std::sin(w0);
        float alpha = sinW0 / (2.0f * BUTTERWORTH_Q);
        float a0 = 1.0f + alpha;

        hpf_.b0 = ((1.0f + cosW0) * 0.5f) / a0;
        hpf_.b1 = -(1.0f + cosW0) / a0;
        hpf_.b2 = ((1.0f + cosW0) * 0.5f) / a0;
        hpf_.a1 = (-2.0f * cosW0) / a0;
        hpf_.a2 = (1.0f - alpha) / a0;
    }

    void recalcLPF()
    {
        if (sampleRate_ <= 0.0)
            return;

        float w0 = 2.0f * PI * lpfFreq_ / static_cast<float>(sampleRate_);
        float cosW0 = std::cos(w0);
        float sinW0 = std::sin(w0);
        float alpha = sinW0 / (2.0f * BUTTERWORTH_Q);
        float a0 = 1.0f + alpha;

        lpf_.b0 = ((1.0f - cosW0) * 0.5f) / a0;
        lpf_.b1 = (1.0f - cosW0) / a0;
        lpf_.b2 = ((1.0f - cosW0) * 0.5f) / a0;
        lpf_.a1 = (-2.0f * cosW0) / a0;
        lpf_.a2 = (1.0f - alpha) / a0;
    }

    double sampleRate_ = 44100.0;
    float hpfFreq_ = HPF_MIN_HZ;
    float lpfFreq_ = LPF_MAX_HZ;
    Biquad hpf_;
    Biquad lpf_;
};

} // namespace ezsqueeze

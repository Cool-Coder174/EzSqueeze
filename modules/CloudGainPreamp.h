#pragma once

#include "../source/dsp/Saturation.h"

#include <cmath>
#include <algorithm>

namespace ezsqueeze
{

/**
 * @brief Clean gain stage with selectable impedance character.
 *
 * Provides 0–30 dB of gain with three impedance voicings:
 *  - **Silicon**: transparent / minimal colouration.
 *  - **Tube**: even-harmonic warmth (2nd, 4th).
 *  - **Transformer**: even + odd harmonics with a subtle +1 dB
 *    low-frequency shelf at 100 Hz.
 */
class CloudGainPreamp
{
public:
    /** Impedance / character type. */
    enum class ImpedanceType
    {
        Silicon,     ///< Clean, minimal harmonics
        Tube,        ///< Even harmonics
        Transformer  ///< Even + odd, subtle LF bump
    };

    /** @brief Set the processing sample rate (needed for transformer LF shelf). */
    void setSampleRate(double sr)
    {
        sampleRate_ = sr;
        recalcLFShelf();
    }

    /**
     * @brief Set the gain amount.
     * @param dB  Gain in dB, clamped to 0–30.
     */
    void setGain(float dB)
    {
        gainDB_ = std::clamp(dB, 0.0f, MAX_GAIN_DB);
        gainLinear_ = std::pow(10.0f, gainDB_ / 20.0f);
    }

    /**
     * @brief Select the impedance character.
     * @param type  Silicon, Tube, or Transformer.
     */
    void setImpedance(ImpedanceType type)
    {
        impedance_ = type;
    }

    /**
     * @brief Process a single sample through the preamp.
     * @param input  Input sample.
     * @return Output sample with gain and character applied.
     */
    float process(float input)
    {
        float out = input * gainLinear_;

        switch (impedance_)
        {
            case ImpedanceType::Silicon:
                break;

            case ImpedanceType::Tube:
            {
                float drive = 1.0f + (gainDB_ / MAX_GAIN_DB) * 0.8f;
                out = Saturation::softClip(out, drive);
                out = Saturation::evenHarmonics(out, TUBE_EVEN_AMOUNT);
                break;
            }

            case ImpedanceType::Transformer:
            {
                float drive = 1.0f + (gainDB_ / MAX_GAIN_DB) * 0.6f;
                out = Saturation::softClip(out, drive);
                out = Saturation::evenHarmonics(out, XFMR_EVEN_AMOUNT);
                out = Saturation::oddHarmonics(out, XFMR_ODD_AMOUNT);
                out = applyLFShelf(out);
                break;
            }
        }

        return out;
    }

    /** @brief Reset the transformer shelf filter state. */
    void reset()
    {
        lfState_ = 0.0f;
    }

private:
    static constexpr float PI = 3.14159265358979323846f;
    static constexpr float MAX_GAIN_DB = 30.0f;
    static constexpr float TUBE_EVEN_AMOUNT = 0.08f;
    static constexpr float XFMR_EVEN_AMOUNT = 0.05f;
    static constexpr float XFMR_ODD_AMOUNT = 0.03f;
    static constexpr float LF_SHELF_FREQ = 100.0f;
    static constexpr float LF_SHELF_GAIN_LINEAR = 0.12202f; // 10^(1/20) - 1 ≈ +1 dB

    void recalcLFShelf()
    {
        if (sampleRate_ <= 0.0)
            return;
        lfCoeff_ = 1.0f - std::exp(-2.0f * PI * LF_SHELF_FREQ / static_cast<float>(sampleRate_));
    }

    float applyLFShelf(float x)
    {
        lfState_ += lfCoeff_ * (x - lfState_);
        return x + lfState_ * LF_SHELF_GAIN_LINEAR;
    }

    double sampleRate_ = 44100.0;
    float gainDB_ = 0.0f;
    float gainLinear_ = 1.0f;
    ImpedanceType impedance_ = ImpedanceType::Silicon;
    float lfState_ = 0.0f;
    float lfCoeff_ = 0.01f;
};

} // namespace ezsqueeze

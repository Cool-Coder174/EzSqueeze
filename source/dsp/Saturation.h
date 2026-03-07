#pragma once

#include <cmath>
#include <algorithm>

namespace ezsqueeze
{

/**
 * @brief Waveshaping functions for harmonic coloration.
 *
 * All functions are stateless and static — they operate on individual
 * samples with no internal memory, making them trivially real-time safe.
 */
class Saturation
{
public:
    /**
     * @brief Symmetric soft-clip via normalised tanh.
     *
     * Output is unity-normalised so that f(±1) = ±1 regardless of drive.
     *
     * @param x     Input sample.
     * @param drive Drive amount (>= 0.1). Higher = more saturation.
     * @return Soft-clipped sample.
     */
    static float softClip(float x, float drive)
    {
        float d = std::max(drive, 0.1f);
        return std::tanh(x * d) / std::tanh(d);
    }

    /**
     * @brief Add even harmonics (2nd, 4th …) via x² waveshaping.
     *
     * Produces asymmetric distortion characteristic of tube-style
     * saturation. Output includes the dry signal.
     *
     * @param x       Input sample.
     * @param amount  Blend amount (0 = clean).
     * @return Sample with added even harmonics.
     */
    static float evenHarmonics(float x, float amount)
    {
        return x + amount * (x * x);
    }

    /**
     * @brief Add odd harmonics (3rd, 5th …) via x³ waveshaping.
     *
     * Produces symmetric distortion. Output includes the dry signal.
     *
     * @param x       Input sample.
     * @param amount  Blend amount (0 = clean).
     * @return Sample with added odd harmonics.
     */
    static float oddHarmonics(float x, float amount)
    {
        return x + amount * (x * x * x);
    }

    /**
     * @brief Asymmetric soft-clip emulating magnetic tape saturation.
     *
     * Positive peaks are driven harder than negative peaks, generating
     * a mix of even and odd harmonics typical of analogue tape.
     *
     * @param x     Input sample.
     * @param drive Drive amount (>= 0.1).
     * @return Tape-saturated sample.
     */
    static float tapeEmulation(float x, float drive)
    {
        float d = std::max(drive, 0.1f);
        if (x >= 0.0f)
        {
            float posD = d * 1.2f;
            return std::tanh(x * posD) / std::tanh(posD);
        }
        else
        {
            float negD = d * 0.85f;
            return std::tanh(x * negD) / std::tanh(negD);
        }
    }

private:
    Saturation() = delete;
};

} // namespace ezsqueeze

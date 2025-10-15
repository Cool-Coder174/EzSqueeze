/**
 * @file Oversampling.h
 * @brief Oversampling engine for alias-free non-linear processing
 * 
 * Provides 2×, 4×, and 8× oversampling with polyphase filtering
 * to prevent aliasing from saturation, compression, and other
 * non-linear processing.
 * 
 * @author Isaac Hernandez
 * @date October 2025
 */

#pragma once

#include <vector>
#include <cmath>
#include <algorithm>

namespace EzSqueeze {
namespace DSP {

/**
 * @brief Oversampling factor
 */
enum class OversamplingFactor
{
    Off = 1,    ///< No oversampling
    X2 = 2,     ///< 2× oversampling
    X4 = 4,     ///< 4× oversampling
    X8 = 8      ///< 8× oversampling
};

/**
 * @class SimpleOversampler
 * @brief Basic oversampling with linear interpolation/decimation
 * 
 * This is a simplified oversampling implementation suitable for
 * prototype/reference. For production, consider using JUCE's
 * juce::dsp::Oversampling with proper polyphase filters.
 * 
 * **Note**: This implementation uses linear interpolation for upsampling
 * and averaging for downsampling. Real polyphase filters would provide
 * better alias rejection.
 * 
 * RT-Safe: Yes (after prepare())
 * Complexity: O(factor) per sample
 */
class SimpleOversampler
{
public:
    SimpleOversampler() = default;
    ~SimpleOversampler() = default;

    /**
     * @brief Prepare oversampler
     * @param sampleRate Base sample rate in Hz
     * @param maxFactor Maximum oversampling factor
     */
    void prepare(double sampleRate, OversamplingFactor maxFactor = OversamplingFactor::X8);

    /**
     * @brief Set oversampling factor
     * @param factor Oversampling factor (Off, X2, X4, X8)
     */
    void setFactor(OversamplingFactor factor);

    /**
     * @brief Process single sample with oversampling
     * @param input Input sample at base sample rate
     * @param processFunc Function to call for each oversampled sample
     * @return Downsampled output sample
     */
    template<typename ProcessFunc>
    float processSample(float input, ProcessFunc&& processFunc)
    {
        if (m_factor == OversamplingFactor::Off)
        {
            return processFunc(input);
        }

        const int factor = static_cast<int>(m_factor);

        // Upsample: linear interpolation from previous to current sample
        float sum = 0.0f;

        for (int i = 0; i < factor; ++i)
        {
            const float t = static_cast<float>(i) / static_cast<float>(factor);
            const float interpolated = m_previousInput + t * (input - m_previousInput);

            // Process at high sample rate
            const float processed = processFunc(interpolated);

            // Accumulate for averaging (simple decimation)
            sum += processed;
        }

        m_previousInput = input;

        // Downsample: simple averaging
        return sum / static_cast<float>(factor);
    }

    /**
     * @brief Reset oversampler state
     */
    void reset();

    /**
     * @brief Get current factor
     * @return Current oversampling factor
     */
    OversamplingFactor getFactor() const { return m_factor; }

    /**
     * @brief Get latency introduced by oversampling
     * @return Latency in samples (at base sample rate)
     */
    int getLatencySamples() const
    {
        // Simple implementation has minimal latency
        // Polyphase filters would add ~8-16 samples per stage
        return (m_factor != OversamplingFactor::Off) ? static_cast<int>(m_factor) : 0;
    }

    /**
     * @brief Get oversampled sample rate
     * @return Effective sample rate after oversampling
     */
    double getOversampledSampleRate() const
    {
        return m_baseSampleRate * static_cast<int>(m_factor);
    }

private:
    OversamplingFactor m_factor = OversamplingFactor::Off;
    double m_baseSampleRate = 48000.0;
    float m_previousInput = 0.0f;
};

/**
 * @class OversamplingController
 * @brief Manages oversampling settings and performance
 * 
 * Provides high-level control over oversampling with
 * automatic quality/performance trade-offs.
 * 
 * RT-Safe: Yes
 * Complexity: O(1)
 */
class OversamplingController
{
public:
    /**
     * @brief Set oversampling factor
     * @param factor Desired oversampling factor
     */
    void setFactor(OversamplingFactor factor);

    /**
     * @brief Enable/disable Eco Mode (forces oversampling off)
     * @param enabled True to enable Eco Mode
     */
    void setEcoMode(bool enabled);

    /**
     * @brief Get effective oversampling factor
     * @return Active factor (accounting for Eco Mode)
     */
    OversamplingFactor getEffectiveFactor() const;

    /**
     * @brief Get CPU cost estimate
     * @return Relative CPU cost (1.0 = no oversampling, 8.0 = 8× oversampling)
     */
    float getCPUCostEstimate() const;

private:
    OversamplingFactor m_requestedFactor = OversamplingFactor::Off;
    bool m_ecoMode = false;
};

} // namespace DSP
} // namespace EzSqueeze


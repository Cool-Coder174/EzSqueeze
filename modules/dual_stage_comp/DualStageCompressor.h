/**
 * @file DualStageCompressor.h
 * @brief Dual-stage serial compression (FET + Opto)
 * 
 * Two compressors in series with different characteristics:
 * Stage 1 (FET-style): Fast, aggressive transient control
 * Stage 2 (Opto-style): Slow, smooth leveling and glue
 * 
 * @author Isaac Hernandez
 * @date October 2025
 */

#pragma once

#include "../../source/dsp/Detector.h"
#include "../../source/dsp/GainComputer.h"
#include "../../source/dsp/EnvelopeFollower.h"

namespace EzSqueeze {
namespace Modules {

/**
 * @brief Compressor stage characteristics
 */
struct CompressorStageConfig
{
    float attackMs;
    float releaseMs;
    float threshold;
    float ratio;
    DSP::KneeMode knee;
    DSP::DetectorMode detectorMode;
};

/**
 * @class DualStageCompressor
 * @brief Two serial compressors with complementary characteristics
 * 
 * The DualStageCompressor chains two compressors in series:
 * - FET Stage: Fast attack/release for transient shaping
 * - Opto Stage: Slow attack/release for smooth leveling
 * 
 * This creates the classic "leveler + polisher" effect, ideal
 * for vocals, bass, and other sources needing both punch and glue.
 * 
 * RT-Safe: Yes (all components are RT-safe)
 * Complexity: O(1) per sample (2× single compressor)
 */
class DualStageCompressor
{
public:
    DualStageCompressor() = default;
    ~DualStageCompressor() = default;

    /**
     * @brief Prepare both stages for processing
     * @param sampleRate Sample rate in Hz
     */
    void prepare(double sampleRate);

    /**
     * @brief Enable/disable dual-stage mode
     * @param enabled True to enable both stages, false for bypass
     */
    void setEnabled(bool enabled);

    /**
     * @brief Configure FET stage parameters
     * @param threshold Threshold in dB
     * @param ratio Compression ratio
     * @param attackMs Attack time in ms
     * @param releaseMs Release time in ms
     */
    void configureFETStage(float threshold, float ratio, 
                          float attackMs = 0.5f, float releaseMs = 50.0f);

    /**
     * @brief Configure Opto stage parameters
     * @param threshold Threshold in dB
     * @param ratio Compression ratio
     * @param attackMs Attack time in ms
     * @param releaseMs Release time in ms
     */
    void configureOptoStage(float threshold, float ratio,
                           float attackMs = 10.0f, float releaseMs = 300.0f);

    /**
     * @brief Process sample through both stages
     * @param input Input sample
     * @param outFetGR FET stage gain reduction (for metering)
     * @param outOptoGR Opto stage gain reduction (for metering)
     * @return Compressed output sample
     */
    float processSample(float input, float& outFetGR, float& outOptoGR);

    /**
     * @brief Reset all stages
     */
    void reset();

    /**
     * @brief Check if dual-stage is enabled
     * @return True if enabled
     */
    bool isEnabled() const { return m_enabled; }

private:
    bool m_enabled = false;
    double m_sampleRate = 48000.0;

    // FET Stage (Stage 1 - Fast)
    DSP::Detector m_fetDetector;
    DSP::GainComputer m_fetGainComp;
    DSP::EnvelopeFollower m_fetEnvelope;

    // Opto Stage (Stage 2 - Slow)
    DSP::Detector m_optoDetector;
    DSP::GainComputer m_optoGainComp;
    DSP::EnvelopeFollower m_optoEnvelope;

    /**
     * @brief Apply gain reduction to sample
     * @param sample Input sample
     * @param grDb Gain reduction in dB (negative)
     * @return Processed sample
     */
    inline float applyGainReduction(float sample, float grDb) const
    {
        const float gain = std::pow(10.0f, grDb / 20.0f);
        return sample * gain;
    }
};

} // namespace Modules
} // namespace EzSqueeze


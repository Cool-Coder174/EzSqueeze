/**
 * @file DualStageCompressor.cpp
 * @brief Implementation of dual-stage serial compression
 */

#include "DualStageCompressor.h"

namespace EzSqueeze {
namespace Modules {

void DualStageCompressor::prepare(double sampleRate)
{
    m_sampleRate = sampleRate;

    // Prepare FET stage (fast, peak detection)
    m_fetDetector.prepare(sampleRate, 1.0f);  // 1ms RMS window
    m_fetDetector.setMode(DSP::DetectorMode::Peak);
    m_fetEnvelope.prepare(sampleRate);
    m_fetGainComp.setKneeMode(DSP::KneeMode::Hard);

    // Prepare Opto stage (slow, RMS detection)
    m_optoDetector.prepare(sampleRate, 10.0f);  // 10ms RMS window
    m_optoDetector.setMode(DSP::DetectorMode::RMS);
    m_optoEnvelope.prepare(sampleRate);
    m_optoGainComp.setKneeMode(DSP::KneeMode::Soft);

    // Set default configurations
    configureFETStage(-12.0f, 8.0f);    // Aggressive: -12dB, 8:1
    configureOptoStage(-18.0f, 3.0f);   // Gentle: -18dB, 3:1

    reset();
}

void DualStageCompressor::setEnabled(bool enabled)
{
    m_enabled = enabled;
}

void DualStageCompressor::configureFETStage(float threshold, float ratio,
                                            float attackMs, float releaseMs)
{
    m_fetGainComp.setThreshold(threshold);
    m_fetGainComp.setRatio(ratio);
    m_fetEnvelope.setAttack(attackMs);
    m_fetEnvelope.setRelease(releaseMs);
}

void DualStageCompressor::configureOptoStage(float threshold, float ratio,
                                             float attackMs, float releaseMs)
{
    m_optoGainComp.setThreshold(threshold);
    m_optoGainComp.setRatio(ratio);
    m_optoEnvelope.setAttack(attackMs);
    m_optoEnvelope.setRelease(releaseMs);
}

float DualStageCompressor::processSample(float input, float& outFetGR, float& outOptoGR)
{
    if (!m_enabled)
    {
        outFetGR = 0.0f;
        outOptoGR = 0.0f;
        return input;
    }

    float signal = input;

    // ========== Stage 1: FET (Fast) ==========
    {
        const float level = m_fetDetector.processSample(signal);
        const float gr = m_fetGainComp.computeGainReduction(level);
        const float smoothGR = m_fetEnvelope.processSample(gr);
        outFetGR = smoothGR;
        signal = applyGainReduction(signal, smoothGR);
    }

    // ========== Stage 2: Opto (Slow) ==========
    {
        const float level = m_optoDetector.processSample(signal);
        const float gr = m_optoGainComp.computeGainReduction(level);
        const float smoothGR = m_optoEnvelope.processSample(gr);
        outOptoGR = smoothGR;
        signal = applyGainReduction(signal, smoothGR);
    }

    return signal;
}

void DualStageCompressor::reset()
{
    m_fetDetector.reset();
    m_fetEnvelope.reset();
    m_optoDetector.reset();
    m_optoEnvelope.reset();
}

} // namespace Modules
} // namespace EzSqueeze


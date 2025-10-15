/**
 * @file LookaheadBuffer.h
 * @brief Delay buffer for lookahead compression
 * 
 * Delays audio to allow detector to "see" incoming peaks before
 * they arrive, enabling zero-overshoot compression with faster
 * attack times and more transparent response.
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
 * @class LookaheadBuffer
 * @brief Circular delay buffer for lookahead processing
 * 
 * The LookaheadBuffer delays the audio signal so the detector
 * can analyze it before it reaches the gain stage. This allows
 * the compressor to react to transients before they occur,
 * preventing overshoot and enabling more transparent compression.
 * 
 * RT-Safe: Yes (after prepare(), no allocations)
 * Complexity: O(1) per sample
 */
class LookaheadBuffer
{
public:
    LookaheadBuffer() = default;
    ~LookaheadBuffer() = default;

    /**
     * @brief Prepare lookahead buffer
     * @param sampleRate Sample rate in Hz
     * @param maxLookaheadMs Maximum lookahead time in ms (typically 0-10ms)
     */
    void prepare(double sampleRate, float maxLookaheadMs = 10.0f);

    /**
     * @brief Set lookahead delay time
     * @param delayMs Delay time in milliseconds (0 to maxLookahead)
     */
    void setDelay(float delayMs);

    /**
     * @brief Process single sample through delay
     * @param input Input sample
     * @return Delayed output sample
     */
    float processSample(float input);

    /**
     * @brief Reset buffer to silence
     */
    void reset();

    /**
     * @brief Get current delay in samples
     * @return Delay length in samples
     */
    int getDelaySamples() const { return m_delaySamples; }

    /**
     * @brief Get current delay in milliseconds
     * @return Delay time in ms
     */
    float getDelayMs() const { return m_delayMs; }

    /**
     * @brief Get latency introduced by this buffer
     * @return Latency in samples (for DAW reporting)
     */
    int getLatencySamples() const { return m_delaySamples; }

private:
    std::vector<float> m_buffer;
    int m_writePos = 0;
    int m_delaySamples = 0;
    float m_delayMs = 0.0f;
    double m_sampleRate = 48000.0;
    int m_maxDelaySamples = 0;
};

} // namespace DSP
} // namespace EzSqueeze

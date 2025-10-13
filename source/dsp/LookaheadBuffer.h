#pragma once

#include <vector>
#include <cstring>

namespace EzSqueeze::DSP {

/**
 * @brief Lookahead buffer for delaying audio to allow peak prediction
 * 
 * Provides configurable delay (0-10ms) with automatic latency reporting.
 * All operations are real-time safe with pre-allocated buffers.
 */
class LookaheadBuffer
{
public:
    /**
     * @brief Constructor
     * @param maxDelayMs Maximum delay in milliseconds
     * @param sampleRate Sample rate in Hz
     */
    LookaheadBuffer(float maxDelayMs = 10.0f, float sampleRate = 44100.0f);

    /**
     * @brief Prepare the buffer for processing
     * @param sampleRate New sample rate
     * @param maxDelayMs Maximum delay in milliseconds
     */
    void prepare(float sampleRate, float maxDelayMs = 10.0f);

    /**
     * @brief Set the lookahead delay
     * @param delayMs Delay in milliseconds (0 to maxDelayMs)
     */
    void setDelay(float delayMs);

    /**
     * @brief Process a single sample
     * @param input Input sample
     * @return Delayed output sample
     */
    float processSample(float input);

    /**
     * @brief Process a block of samples
     * @param input Input buffer
     * @param output Output buffer
     * @param numSamples Number of samples to process
     */
    void processBlock(const float* input, float* output, int numSamples);

    /**
     * @brief Reset the buffer state (clear all samples)
     */
    void reset();

    /**
     * @brief Get current delay in milliseconds
     * @return Delay in milliseconds
     */
    float getDelayMs() const { return delayMs_; }

    /**
     * @brief Get current delay in samples
     * @return Delay in samples
     */
    int getDelaySamples() const { return delaySamples_; }

    /**
     * @brief Get maximum possible delay in milliseconds
     * @return Maximum delay in milliseconds
     */
    float getMaxDelayMs() const { return maxDelayMs_; }

    /**
     * @brief Get maximum possible delay in samples
     * @return Maximum delay in samples
     */
    int getMaxDelaySamples() const { return maxDelaySamples_; }

    /**
     * @brief Check if buffer is ready for processing
     * @return True if buffer is prepared and ready
     */
    bool isReady() const { return !buffer_.empty(); }

private:
    std::vector<float> buffer_;
    int writePos_;
    int delaySamples_;
    float delayMs_;
    float sampleRate_;
    float maxDelayMs_;
    int maxDelaySamples_;

    /**
     * @brief Calculate delay in samples from milliseconds
     * @param delayMs Delay in milliseconds
     * @return Delay in samples
     */
    int msToSamples(float delayMs) const;

    /**
     * @brief Calculate delay in milliseconds from samples
     * @param delaySamples Delay in samples
     * @return Delay in milliseconds
     */
    float samplesToMs(int delaySamples) const;

    /**
     * @brief Clamp delay value to valid range
     * @param delayMs Delay in milliseconds
     * @return Clamped delay in milliseconds
     */
    float clampDelay(float delayMs) const;
};

} // namespace EzSqueeze::DSP
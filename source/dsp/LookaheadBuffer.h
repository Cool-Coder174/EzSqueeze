#pragma once

#include <array>
#include <algorithm>
#include <cmath>

namespace ezsqueeze
{

/**
 * @brief Circular delay buffer for compressor lookahead (0–10 ms).
 *
 * Pre-allocates a fixed buffer large enough for 10 ms at 192 kHz
 * (1920 samples) on the stack. Delay time is set in milliseconds
 * and converted to integer samples for the host latency report.
 */
class LookaheadBuffer
{
public:
    /** Maximum lookahead: 10 ms at 192 kHz. */
    static constexpr int MAX_DELAY_SAMPLES = 1920;

    /**
     * @brief Set the lookahead delay.
     * @param ms         Delay in milliseconds (0–10).
     * @param sampleRate Current sample rate in Hz.
     */
    void setDelay(float ms, double sampleRate)
    {
        float clamped = std::clamp(ms, 0.0f, 10.0f);
        delaySamples_ = static_cast<int>(std::round(clamped * 0.001f * sampleRate));
        delaySamples_ = std::clamp(delaySamples_, 0, MAX_DELAY_SAMPLES - 1);
    }

    /**
     * @brief Push a sample in and return the delayed sample.
     * @param input  Input sample.
     * @return Sample delayed by the configured amount.
     */
    float process(float input)
    {
        buffer_[writeIndex_] = input;

        int readIndex = writeIndex_ - delaySamples_;
        if (readIndex < 0)
            readIndex += MAX_DELAY_SAMPLES;

        float output = buffer_[static_cast<size_t>(readIndex)];

        writeIndex_ = (writeIndex_ + 1) % MAX_DELAY_SAMPLES;

        return output;
    }

    /**
     * @brief Return the current delay in samples for host latency reporting.
     * @return Delay length in samples.
     */
    int getLatencySamples() const
    {
        return delaySamples_;
    }

    /** @brief Zero the buffer and reset the write head. */
    void reset()
    {
        buffer_.fill(0.0f);
        writeIndex_ = 0;
    }

private:
    std::array<float, MAX_DELAY_SAMPLES> buffer_ {};
    int writeIndex_ = 0;
    int delaySamples_ = 0;
};

} // namespace ezsqueeze

#pragma once

#include <vector>
#include <cstddef>
#include <cstdint>
#include <algorithm>
#include "Utilities.h"

namespace ezsqueeze::dsp {

class LookaheadBuffer {
public:
    LookaheadBuffer() = default;

    void prepare(const double newSampleRate, const std::size_t numChannels) {
        sampleRate = newSampleRate;
        channels = std::max<std::size_t>(1, numChannels);
        setDelayMilliseconds(delayMilliseconds);
        reset();
    }

    void reset() {
        for (auto &buffer : channelBuffers) {
            std::fill(buffer.begin(), buffer.end(), 0.0f);
        }
        writeIndex = 0;
    }

    void setDelayMilliseconds(const float delayMs) {
        delayMilliseconds = std::max(delayMs, 0.0f);
        const int newDelaySamples = static_cast<int>(delayMilliseconds * 0.001 * sampleRate + 0.5);
        delaySamples = std::max(0, newDelaySamples);
        // Use ring size of delaySamples + 1 to achieve exact delay of N samples
        const std::size_t ringSize = static_cast<std::size_t>(std::max(1, delaySamples + 1));
        channelBuffers.resize(channels);
        for (auto &buffer : channelBuffers) {
            buffer.assign(ringSize, 0.0f);
        }
        writeIndex = 0;
    }

    float processSample(const std::size_t channelIndex, const float inputSample) noexcept {
        if (delaySamples == 0) {
            return inputSample; // bypass when no delay
        }
        auto &buffer = channelBuffers[channelIndex % channels];
        const std::size_t ringSize = buffer.size();
        const std::size_t readIndex = (writeIndex + ringSize - static_cast<std::size_t>(delaySamples % static_cast<int>(ringSize))) % ringSize;
        const float output = buffer[readIndex];
        buffer[writeIndex] = inputSample;
        writeIndex = (writeIndex + 1) % ringSize;
        return output;
    }

    int getLatencySamples() const noexcept { return delaySamples; }

    float getLatencyMilliseconds() const noexcept {
        return static_cast<float>(delaySamples) * 1000.0f / static_cast<float>(sampleRate);
    }

private:
    double sampleRate { 44100.0 };
    std::size_t channels { 2 };
    float delayMilliseconds { 0.0f };
    int delaySamples { 0 };
    std::vector<std::vector<float>> channelBuffers {};
    std::size_t writeIndex { 0 };
};

} // namespace ezsqueeze::dsp

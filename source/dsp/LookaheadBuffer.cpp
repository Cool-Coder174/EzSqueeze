#include "LookaheadBuffer.h"

namespace ezsqueeze::dsp {

void LookaheadBuffer::prepare(double newSampleRate, int numChannels) {
    sampleRate = newSampleRate;
    channels = std::max(1, numChannels);
    allocate();
    reset();
}

void LookaheadBuffer::reset() {
    for (int ch = 0; ch < channels; ++ch) {
        std::fill(ringBuffer[ch].begin(), ringBuffer[ch].end(), 0.0f);
        writePositions[ch] = 0;
    }
}

void LookaheadBuffer::setDelayMs(float newDelayMs) {
    delayMs = std::max(0.0f, newDelayMs);
    lookaheadSamples = static_cast<int>(delayMs * 0.001f * static_cast<float>(sampleRate));
    allocate();
}

void LookaheadBuffer::process(float** audio, int numChannelsIn, int numSamples) {
    const int chs = std::min(channels, std::max(1, numChannelsIn));
    if (lookaheadSamples <= 0) {
        return; // no-op
    }

    // Ensure buffer is at least lookaheadSamples length
    if (ringBuffer.empty() || static_cast<int>(ringBuffer[0].size()) != lookaheadSamples) {
        allocate();
    }

    for (int ch = 0; ch < chs; ++ch) {
        float* channelData = audio[ch];
        std::vector<float>& buffer = ringBuffer[ch];
        int& writePos = writePositions[ch];

        for (int n = 0; n < numSamples; ++n) {
            const int readPos = writePos;
            const float delayedSample = buffer[readPos];
            buffer[writePos] = channelData[n];
            writePos = (writePos + 1) % lookaheadSamples;
            channelData[n] = delayedSample;
        }
    }
}

void LookaheadBuffer::allocate() {
    const int size = std::max(1, lookaheadSamples);
    ringBuffer.assign(channels, std::vector<float>(size, 0.0f));
    writePositions.assign(channels, 0);
}

} // namespace ezsqueeze::dsp

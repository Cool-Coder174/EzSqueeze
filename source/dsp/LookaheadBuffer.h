#pragma once
#include <vector>
#include <cstdint>
#include <algorithm>

namespace ezsqueeze::dsp {

class LookaheadBuffer {
public:
    void prepare(double newSampleRate, int numChannels);
    void reset();

    void setDelayMs(float newDelayMs);

    int getLatencySamples() const { return lookaheadSamples; }
    float getDelayMs() const { return delayMs; }

    // In-place delay of audio block per channel
    // audio[ch] points to an array of numSamples
    void process(float** audio, int numChannels, int numSamples);

private:
    void allocate();

    double sampleRate { 48000.0 };
    int channels { 0 };

    float delayMs { 0.0f };
    int lookaheadSamples { 0 };

    std::vector<std::vector<float>> ringBuffer; // [channel][index]
    std::vector<int> writePositions;            // [channel]
};

} // namespace ezsqueeze::dsp

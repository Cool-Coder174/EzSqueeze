#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>
#include <cmath>
#include <vector>
#include "Detector.h"
#include "GainComputer.h"
#include "EnvelopeFollower.h"
#include "LookaheadBuffer.h"
#include "SidechainFilter.h"
#include "StereoLink.h"
#include "AutoMakeup.h"

using namespace ezsqueeze;

static constexpr double kSampleRate = 44100.0;

static std::vector<float> generateTestSignal(int numSamples) {
    std::vector<float> signal(numSamples);
    for (int i = 0; i < numSamples; ++i)
        signal[i] = std::sin(2.0f * static_cast<float>(M_PI) * 440.0f * i / static_cast<float>(kSampleRate)) * 0.8f;
    return signal;
}

TEST_CASE("Detector performance", "[benchmark][detector]") {
    auto signal = generateTestSignal(44100);

    BENCHMARK("Peak detector - 1 second @ 44.1kHz") {
        Detector det;
        det.prepare(kSampleRate, 512);
        det.setMode(Detector::Mode::Peak);

        float result = 0.0f;
        for (auto sample : signal)
            result = det.process(sample);
        return result;
    };

    BENCHMARK("RMS detector - 1 second @ 44.1kHz") {
        Detector det;
        det.prepare(kSampleRate, 512);
        det.setMode(Detector::Mode::RMS);

        float result = 0.0f;
        for (auto sample : signal)
            result = det.process(sample);
        return result;
    };
}

TEST_CASE("GainComputer performance", "[benchmark][gaincomputer]") {
    std::vector<float> levels(44100);
    for (int i = 0; i < 44100; ++i)
        levels[i] = -40.0f + 40.0f * std::sin(2.0f * static_cast<float>(M_PI) * 2.0f * i / 44100.0f);

    BENCHMARK("GainComputer hard knee - 1 second") {
        GainComputer gc;
        gc.setThreshold(-20.0f);
        gc.setRatio(4.0f);
        gc.setKneeWidth(0.0f);

        float result = 0.0f;
        for (auto db : levels)
            result = gc.computeGainReduction(db);
        return result;
    };

    BENCHMARK("GainComputer soft knee - 1 second") {
        GainComputer gc;
        gc.setThreshold(-20.0f);
        gc.setRatio(4.0f);
        gc.setKneeWidth(6.0f);

        float result = 0.0f;
        for (auto db : levels)
            result = gc.computeGainReduction(db);
        return result;
    };
}

TEST_CASE("EnvelopeFollower performance", "[benchmark][envelope]") {
    std::vector<float> grValues(44100);
    for (int i = 0; i < 44100; ++i)
        grValues[i] = std::abs(std::sin(2.0f * static_cast<float>(M_PI) * 4.0f * i / 44100.0f)) * 20.0f;

    BENCHMARK("EnvelopeFollower - 1 second @ 44.1kHz") {
        EnvelopeFollower env;
        env.prepare(kSampleRate);
        env.setAttack(10.0f);
        env.setRelease(100.0f);

        float result = 0.0f;
        for (auto val : grValues)
            result = env.process(val);
        return result;
    };
}

TEST_CASE("Full chain performance", "[benchmark][chain]") {
    auto signal = generateTestSignal(44100);

    BENCHMARK("Full compressor chain - 1 second mono @ 44.1kHz") {
        Detector det;
        det.prepare(kSampleRate, 512);
        det.setMode(Detector::Mode::Peak);

        GainComputer gc;
        gc.setThreshold(-20.0f);
        gc.setRatio(4.0f);
        gc.setKneeWidth(6.0f);

        EnvelopeFollower env;
        env.prepare(kSampleRate);
        env.setAttack(10.0f);
        env.setRelease(100.0f);

        LookaheadBuffer la;
        la.setDelay(5.0f, kSampleRate);

        float result = 0.0f;
        for (auto sample : signal) {
            float level = det.process(sample);
            float db = det.toDecibels(level);
            float gr = gc.computeGainReduction(db);
            float smoothGR = env.process(std::abs(gr));
            float delayed = la.process(sample);
            float gainLin = std::pow(10.0f, -smoothGR / 20.0f);
            result = delayed * gainLin;
        }
        return result;
    };
}

TEST_CASE("Block size performance comparison", "[benchmark][blocksize]") {
    auto signal = generateTestSignal(44100);
    const int blockSizes[] = {32, 64, 128, 256, 512, 1024};

    for (int blockSize : blockSizes) {
        DYNAMIC_SECTION("Block size: " << blockSize) {
            BENCHMARK("Process 1s in blocks of " + std::to_string(blockSize)) {
                Detector det;
                det.prepare(kSampleRate, blockSize);
                det.setMode(Detector::Mode::Peak);

                GainComputer gc;
                gc.setThreshold(-20.0f);
                gc.setRatio(4.0f);
                gc.setKneeWidth(0.0f);

                EnvelopeFollower env;
                env.prepare(kSampleRate);
                env.setAttack(10.0f);
                env.setRelease(100.0f);

                float result = 0.0f;
                int numBlocks = 44100 / blockSize;
                for (int b = 0; b < numBlocks; ++b) {
                    int offset = b * blockSize;
                    for (int i = 0; i < blockSize; ++i) {
                        float level = det.process(signal[offset + i]);
                        float db = det.toDecibels(level);
                        float gr = gc.computeGainReduction(db);
                        result = env.process(std::abs(gr));
                    }
                }
                return result;
            };
        }
    }
}

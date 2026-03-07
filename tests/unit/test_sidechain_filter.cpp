#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <cmath>
#include <vector>
#include "SidechainFilter.h"

using namespace ezsqueeze;
using Catch::Matchers::WithinAbs;

static constexpr double kSampleRate = 44100.0;
static constexpr int kSettleTime = 44100;

static float measureSteadyStateLevel(SidechainFilter& filter, float freq, double sampleRate, int duration) {
    float peak = 0.0f;
    int startMeasure = duration / 2;

    for (int i = 0; i < duration; ++i) {
        float sample = std::sin(2.0f * static_cast<float>(M_PI) * freq * i / static_cast<float>(sampleRate));
        float out = filter.process(sample);
        if (i >= startMeasure)
            peak = std::max(peak, std::abs(out));
    }
    return peak;
}

TEST_CASE("HPF passes high frequencies, attenuates low", "[sidechain][hpf]") {
    SidechainFilter filter;
    filter.prepare(kSampleRate);
    filter.setHPF(500.0f);
    filter.setLPF(20000.0f);

    float lowLevel  = measureSteadyStateLevel(filter, 50.0f, kSampleRate, kSettleTime);
    filter.reset();
    float highLevel = measureSteadyStateLevel(filter, 5000.0f, kSampleRate, kSettleTime);

    REQUIRE(highLevel > lowLevel * 3.0f);
    REQUIRE(highLevel > 0.7f);
    REQUIRE(lowLevel < 0.3f);
}

TEST_CASE("LPF passes low frequencies, attenuates high", "[sidechain][lpf]") {
    SidechainFilter filter;
    filter.prepare(kSampleRate);
    filter.setHPF(20.0f);
    filter.setLPF(1000.0f);

    float lowLevel  = measureSteadyStateLevel(filter, 100.0f, kSampleRate, kSettleTime);
    filter.reset();
    float highLevel = measureSteadyStateLevel(filter, 10000.0f, kSampleRate, kSettleTime);

    REQUIRE(lowLevel > highLevel * 3.0f);
    REQUIRE(lowLevel > 0.7f);
    REQUIRE(highLevel < 0.3f);
}

TEST_CASE("At cutoff frequency: approximately -3 dB", "[sidechain][cutoff]") {
    SECTION("HPF at cutoff") {
        SidechainFilter filter;
        filter.prepare(kSampleRate);
        filter.setHPF(1000.0f);
        filter.setLPF(20000.0f);

        float levelAtCutoff = measureSteadyStateLevel(filter, 1000.0f, kSampleRate, kSettleTime);
        float levelDb = 20.0f * std::log10(std::max(levelAtCutoff, 1e-6f));

        REQUIRE_THAT(levelDb, WithinAbs(-3.0, 1.5));
    }

    SECTION("LPF at cutoff") {
        SidechainFilter filter;
        filter.prepare(kSampleRate);
        filter.setHPF(20.0f);
        filter.setLPF(2000.0f);

        float levelAtCutoff = measureSteadyStateLevel(filter, 2000.0f, kSampleRate, kSettleTime);
        float levelDb = 20.0f * std::log10(std::max(levelAtCutoff, 1e-6f));

        REQUIRE_THAT(levelDb, WithinAbs(-3.0, 1.5));
    }
}

TEST_CASE("Bypass (HPF=20, LPF=16000) passes signal through", "[sidechain][bypass]") {
    SidechainFilter filter;
    filter.prepare(kSampleRate);
    filter.setHPF(20.0f);
    filter.setLPF(16000.0f);

    float testFreqs[] = {100.0f, 440.0f, 1000.0f, 5000.0f, 10000.0f};

    for (float freq : testFreqs) {
        DYNAMIC_SECTION("Frequency: " << freq << " Hz") {
            filter.reset();
            float level = measureSteadyStateLevel(filter, freq, kSampleRate, kSettleTime);
            REQUIRE(level > 0.85f);
        }
    }
}

TEST_CASE("HPF and LPF combined create bandpass", "[sidechain][bandpass]") {
    SidechainFilter filter;
    filter.prepare(kSampleRate);
    filter.setHPF(500.0f);
    filter.setLPF(2000.0f);

    float belowLevel = measureSteadyStateLevel(filter, 50.0f, kSampleRate, kSettleTime);
    filter.reset();
    float midLevel = measureSteadyStateLevel(filter, 1000.0f, kSampleRate, kSettleTime);
    filter.reset();
    float aboveLevel = measureSteadyStateLevel(filter, 15000.0f, kSampleRate, kSettleTime);

    REQUIRE(midLevel > belowLevel * 2.0f);
    REQUIRE(midLevel > aboveLevel * 2.0f);
}

TEST_CASE("Filter works at different sample rates", "[sidechain][samplerate]") {
    double sampleRates[] = {44100.0, 48000.0, 88200.0, 96000.0};

    for (double sr : sampleRates) {
        DYNAMIC_SECTION("Sample rate: " << sr) {
            SidechainFilter filter;
            filter.prepare(sr);
            filter.setHPF(500.0f);
            filter.setLPF(20000.0f);

            int settle = static_cast<int>(sr);
            float highLevel = measureSteadyStateLevel(filter, 5000.0f, sr, settle);
            REQUIRE(highLevel > 0.7f);
        }
    }
}

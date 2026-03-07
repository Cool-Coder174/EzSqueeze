#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <cmath>
#include "Detector.h"

using namespace ezsqueeze;
using Catch::Matchers::WithinAbs;

static constexpr double kSampleRate = 44100.0;
static constexpr int kBlockSize = 512;

TEST_CASE("Peak detector tracks instantaneous peaks", "[detector][peak]") {
    Detector det;
    det.prepare(kSampleRate, kBlockSize);
    det.setMode(Detector::Mode::Peak);

    SECTION("Returns peak of a single positive sample") {
        float result = det.process(0.75f);
        REQUIRE_THAT(result, WithinAbs(0.75, 0.001));
    }

    SECTION("Returns peak of a negative sample (absolute value)") {
        float result = det.process(-0.9f);
        REQUIRE_THAT(result, WithinAbs(0.9, 0.001));
    }

    SECTION("Holds peak across consecutive samples") {
        det.process(0.5f);
        det.process(0.8f);
        float result = det.process(0.3f);
        REQUIRE(result >= 0.3f);
    }

    SECTION("Tracks rising peaks accurately") {
        for (int i = 0; i < 100; ++i)
            det.process(0.1f);

        float result = det.process(1.0f);
        REQUIRE_THAT(result, WithinAbs(1.0, 0.001));
    }
}

TEST_CASE("RMS detector averages correctly over window", "[detector][rms]") {
    Detector det;
    det.prepare(kSampleRate, kBlockSize);
    det.setMode(Detector::Mode::RMS);

    SECTION("Constant input converges to that value") {
        float result = 0.0f;
        for (int i = 0; i < 4410; ++i)
            result = det.process(0.5f);

        REQUIRE_THAT(result, WithinAbs(0.5, 0.02));
    }

    SECTION("RMS of sine wave converges to 1/sqrt(2) of amplitude") {
        float result = 0.0f;
        const float amplitude = 1.0f;
        const float freq = 1000.0f;
        for (int i = 0; i < 44100; ++i) {
            float sample = amplitude * std::sin(2.0f * M_PI * freq * i / kSampleRate);
            result = det.process(sample);
        }
        float expected = amplitude / std::sqrt(2.0f);
        REQUIRE_THAT(result, WithinAbs(expected, 0.05));
    }
}

TEST_CASE("dB conversion is accurate", "[detector][dB]") {
    Detector det;
    det.prepare(kSampleRate, kBlockSize);
    det.setMode(Detector::Mode::Peak);

    SECTION("Unity (1.0) maps to 0 dB") {
        float level = det.process(1.0f);
        float db = det.toDecibels(level);
        REQUIRE_THAT(db, WithinAbs(0.0, 0.01));
    }

    SECTION("0.5 maps to approximately -6.02 dB") {
        float level = det.process(0.5f);
        float db = det.toDecibels(level);
        REQUIRE_THAT(db, WithinAbs(-6.02, 0.01));
    }

    SECTION("0.001 maps to approximately -60 dB") {
        float level = det.process(0.001f);
        float db = det.toDecibels(level);
        REQUIRE_THAT(db, WithinAbs(-60.0, 0.1));
    }
}

TEST_CASE("Zero input returns floor dB value", "[detector][floor]") {
    Detector det;
    det.prepare(kSampleRate, kBlockSize);
    det.setMode(Detector::Mode::Peak);

    float level = det.process(0.0f);
    float db = det.toDecibels(level);
    REQUIRE(db <= -120.0f);
}

TEST_CASE("Detector works at different sample rates", "[detector][samplerate]") {
    const double sampleRates[] = {22050.0, 44100.0, 48000.0, 88200.0, 96000.0, 192000.0};

    for (double sr : sampleRates) {
        DYNAMIC_SECTION("Sample rate: " << sr) {
            Detector det;
            det.prepare(sr, kBlockSize);
            det.setMode(Detector::Mode::Peak);

            float result = det.process(0.75f);
            REQUIRE_THAT(result, WithinAbs(0.75, 0.001));

            det.setMode(Detector::Mode::RMS);
            float rmsResult = 0.0f;
            int numSamples = static_cast<int>(sr * 0.1);
            for (int i = 0; i < numSamples; ++i)
                rmsResult = det.process(0.5f);

            REQUIRE_THAT(rmsResult, WithinAbs(0.5, 0.05));
        }
    }
}

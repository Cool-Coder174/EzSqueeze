#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <cmath>
#include <vector>
#include "LookaheadBuffer.h"

using namespace ezsqueeze;
using Catch::Matchers::WithinAbs;

static constexpr double kSampleRate = 44100.0;

TEST_CASE("Delay is correct number of samples for given ms/sampleRate", "[lookahead][delay]") {
    LookaheadBuffer buf;

    SECTION("5 ms at 44100 Hz = 220 samples") {
        buf.setDelay(5.0f, kSampleRate);
        REQUIRE(buf.getLatencySamples() == 220);
    }

    SECTION("10 ms at 48000 Hz = 480 samples") {
        buf.setDelay(10.0f, 48000.0);
        REQUIRE(buf.getLatencySamples() == 480);
    }

    SECTION("1 ms at 96000 Hz = 96 samples") {
        buf.setDelay(1.0f, 96000.0);
        REQUIRE(buf.getLatencySamples() == 96);
    }

    SECTION("2.5 ms at 44100 Hz = 110 samples") {
        buf.setDelay(2.5f, kSampleRate);
        REQUIRE(buf.getLatencySamples() == 110);
    }
}

TEST_CASE("Signal passes through unmodified, just delayed", "[lookahead][passthrough]") {
    LookaheadBuffer buf;
    float delayMs = 5.0f;
    buf.setDelay(delayMs, kSampleRate);
    int delaySamples = buf.getLatencySamples();

    std::vector<float> input(delaySamples + 1000);
    for (size_t i = 0; i < input.size(); ++i)
        input[i] = std::sin(2.0f * M_PI * 440.0f * i / kSampleRate);

    std::vector<float> output(input.size());
    for (size_t i = 0; i < input.size(); ++i)
        output[i] = buf.process(input[i]);

    for (int i = delaySamples; i < static_cast<int>(input.size()); ++i) {
        REQUIRE_THAT(output[i], WithinAbs(input[i - delaySamples], 0.0001));
    }
}

TEST_CASE("Zero delay = pass-through", "[lookahead][zero]") {
    LookaheadBuffer buf;
    buf.setDelay(0.0f, kSampleRate);

    REQUIRE(buf.getLatencySamples() == 0);

    for (int i = 0; i < 100; ++i) {
        float sample = static_cast<float>(i) / 100.0f;
        REQUIRE_THAT(buf.process(sample), WithinAbs(sample, 0.0001));
    }
}

TEST_CASE("Latency reporting matches actual delay", "[lookahead][latency]") {
    LookaheadBuffer buf;

    const float delayValues[] = {0.0f, 1.0f, 2.0f, 5.0f, 10.0f, 20.0f};
    const double sampleRates[] = {44100.0, 48000.0, 96000.0};

    for (double sr : sampleRates) {
        for (float delayMs : delayValues) {
            DYNAMIC_SECTION("Delay " << delayMs << " ms @ " << sr << " Hz") {
                buf.setDelay(delayMs, sr);

                int reportedLatency = buf.getLatencySamples();
                int expectedSamples = static_cast<int>(delayMs * 0.001f * sr);

                REQUIRE(reportedLatency == expectedSamples);

                if (expectedSamples > 0) {
                    for (int i = 0; i < expectedSamples; ++i)
                        buf.process(0.0f);

                    buf.process(1.0f);
                    float out = 0.0f;
                    out = buf.process(0.0f);
                    bool foundPulse = (std::abs(out - 1.0f) < 0.0001f);

                    if (!foundPulse) {
                        for (int i = 0; i < expectedSamples; ++i) {
                            out = buf.process(0.0f);
                            if (std::abs(out - 1.0f) < 0.0001f) {
                                foundPulse = true;
                                break;
                            }
                        }
                    }
                    REQUIRE(foundPulse);
                }
            }
        }
    }
}

TEST_CASE("Buffer handles impulse correctly", "[lookahead][impulse]") {
    LookaheadBuffer buf;
    buf.setDelay(5.0f, kSampleRate);
    int delaySamples = buf.getLatencySamples();

    for (int i = 0; i < delaySamples; ++i) {
        float out = buf.process(0.0f);
        REQUIRE_THAT(out, WithinAbs(0.0, 0.0001));
    }

    buf.process(1.0f);

    for (int i = 0; i < delaySamples - 1; ++i)
        buf.process(0.0f);

    float impulseOut = buf.process(0.0f);
    REQUIRE_THAT(impulseOut, WithinAbs(1.0, 0.0001));
}

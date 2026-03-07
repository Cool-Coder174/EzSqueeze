#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <cmath>
#include "EnvelopeFollower.h"

using namespace ezsqueeze;
using Catch::Matchers::WithinAbs;

static constexpr double kSampleRate = 44100.0;

TEST_CASE("Attack: step input converges to ~63% within attack time", "[envelope][attack]") {
    EnvelopeFollower env;
    env.prepare(kSampleRate);
    env.setAttack(10.0f);
    env.setRelease(100.0f);

    int attackSamples = static_cast<int>(10.0f * 0.001f * kSampleRate);

    float output = 0.0f;
    for (int i = 0; i < attackSamples; ++i)
        output = env.process(1.0f);

    REQUIRE_THAT(output, WithinAbs(0.632, 0.05));
}

TEST_CASE("Release: step-down converges to ~37% within release time", "[envelope][release]") {
    EnvelopeFollower env;
    env.prepare(kSampleRate);
    env.setAttack(0.01f);
    env.setRelease(50.0f);

    for (int i = 0; i < 44100; ++i)
        env.process(1.0f);

    int releaseSamples = static_cast<int>(50.0f * 0.001f * kSampleRate);

    float output = 0.0f;
    for (int i = 0; i < releaseSamples; ++i)
        output = env.process(0.0f);

    REQUIRE_THAT(output, WithinAbs(0.368, 0.05));
}

TEST_CASE("Fast attack + slow release: typical compressor behavior", "[envelope][combined]") {
    EnvelopeFollower env;
    env.prepare(kSampleRate);
    env.setAttack(1.0f);
    env.setRelease(200.0f);

    for (int i = 0; i < 441; ++i)
        env.process(1.0f);

    float peakResponse = env.process(1.0f);
    REQUIRE(peakResponse > 0.9f);

    float output = peakResponse;
    for (int i = 0; i < 441; ++i)
        output = env.process(0.0f);

    REQUIRE(output > 0.5f);
}

TEST_CASE("Envelope tracks sustained signal fully", "[envelope][sustain]") {
    EnvelopeFollower env;
    env.prepare(kSampleRate);
    env.setAttack(5.0f);
    env.setRelease(100.0f);

    float output = 0.0f;
    for (int i = 0; i < 44100; ++i)
        output = env.process(0.8f);

    REQUIRE_THAT(output, WithinAbs(0.8, 0.01));
}

TEST_CASE("Different sample rates produce consistent timing", "[envelope][samplerate]") {
    const double sampleRates[] = {44100.0, 48000.0, 88200.0, 96000.0};
    const float attackMs = 10.0f;

    float referenceOutput = 0.0f;

    for (size_t sr_idx = 0; sr_idx < 4; ++sr_idx) {
        double sr = sampleRates[sr_idx];

        DYNAMIC_SECTION("Sample rate: " << sr) {
            EnvelopeFollower env;
            env.prepare(sr);
            env.setAttack(attackMs);
            env.setRelease(100.0f);

            int attackSamples = static_cast<int>(attackMs * 0.001 * sr);
            float output = 0.0f;
            for (int i = 0; i < attackSamples; ++i)
                output = env.process(1.0f);

            if (sr_idx == 0) {
                referenceOutput = output;
            } else {
                REQUIRE_THAT(output, WithinAbs(referenceOutput, 0.05));
            }

            REQUIRE_THAT(output, WithinAbs(0.632, 0.05));
        }
    }
}

TEST_CASE("Zero attack time gives near-instant response", "[envelope][edge]") {
    EnvelopeFollower env;
    env.prepare(kSampleRate);
    env.setAttack(0.01f);
    env.setRelease(100.0f);

    float output = env.process(1.0f);
    REQUIRE(output > 0.5f);
}

TEST_CASE("Very long release holds level", "[envelope][edge]") {
    EnvelopeFollower env;
    env.prepare(kSampleRate);
    env.setAttack(1.0f);
    env.setRelease(5000.0f);

    for (int i = 0; i < 44100; ++i)
        env.process(1.0f);

    float output = 0.0f;
    for (int i = 0; i < 4410; ++i)
        output = env.process(0.0f);

    REQUIRE(output > 0.8f);
}

#include <catch2/catch_test_macros.hpp>
#include "dsp/EnvelopeFollower.h"

using namespace EzSqueeze::DSP;

TEST_CASE("EnvelopeFollower attack/release behavior", "[envelope]") {
    EnvelopeFollower env;
    env.prepare(48000.0);
    env.setAttack(10.0f);
    env.setRelease(100.0f);

    // Rising input should use attack coeff (faster change)
    float y1 = env.processSample(-6.0f);
    float y2 = env.processSample(-12.0f); // More negative = more GR -> rising
    REQUIRE(y2 < y1);

    // Falling input should use release coeff (slower change towards 0)
    float y3 = env.processSample(0.0f);
    REQUIRE(y3 > y2);
}

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <cmath>
#include "StereoLink.h"

using namespace ezsqueeze;
using Catch::Matchers::WithinAbs;

TEST_CASE("100% stereo link: both channels get max level", "[stereo][link]") {
    StereoLink sl;
    sl.setLinkAmount(1.0f);

    float leftLevel  = 0.3f;
    float rightLevel = 0.8f;
    float maxLevel = std::max(leftLevel, rightLevel);

    sl.processLink(leftLevel, rightLevel);

    REQUIRE_THAT(leftLevel, WithinAbs(maxLevel, 0.001));
    REQUIRE_THAT(rightLevel, WithinAbs(maxLevel, 0.001));
}

TEST_CASE("0% stereo link: channels independent", "[stereo][link]") {
    StereoLink sl;
    sl.setLinkAmount(0.0f);

    float leftLevel  = 0.3f;
    float rightLevel = 0.8f;
    float origL = leftLevel;
    float origR = rightLevel;

    sl.processLink(leftLevel, rightLevel);

    REQUIRE_THAT(leftLevel, WithinAbs(origL, 0.001));
    REQUIRE_THAT(rightLevel, WithinAbs(origR, 0.001));
}

TEST_CASE("50% stereo link: blended levels", "[stereo][link]") {
    StereoLink sl;
    sl.setLinkAmount(0.5f);

    float leftLevel  = 0.2f;
    float rightLevel = 0.8f;
    float maxLevel = std::max(leftLevel, rightLevel);

    sl.processLink(leftLevel, rightLevel);

    REQUIRE(leftLevel > 0.2f - 0.001f);
    REQUIRE(leftLevel < maxLevel + 0.001f);
    REQUIRE(rightLevel > 0.8f - 0.001f);
    REQUIRE(rightLevel <= maxLevel + 0.001f);
}

TEST_CASE("Equal levels: link amount doesn't matter", "[stereo][link]") {
    StereoLink sl;

    float level = 0.6f;
    float linkAmounts[] = {0.0f, 0.25f, 0.5f, 0.75f, 1.0f};

    for (float link : linkAmounts) {
        DYNAMIC_SECTION("Link: " << link * 100 << "%") {
            sl.setLinkAmount(link);
            float l = level, r = level;
            sl.processLink(l, r);
            REQUIRE_THAT(l, WithinAbs(level, 0.001));
            REQUIRE_THAT(r, WithinAbs(level, 0.001));
        }
    }
}

TEST_CASE("M/S encode/decode roundtrip preserves original", "[stereo][ms]") {
    StereoLink sl;

    SECTION("Simple L/R values") {
        float left = 0.7f;
        float right = 0.3f;
        float mid, side, lOut, rOut;
        sl.encodeMidSide(left, right, mid, side);
        sl.decodeMidSide(mid, side, lOut, rOut);

        REQUIRE_THAT(lOut, WithinAbs(left, 0.0001));
        REQUIRE_THAT(rOut, WithinAbs(right, 0.0001));
    }

    SECTION("Mono signal: side is zero") {
        float left = 0.5f;
        float right = 0.5f;
        float mid, side;
        sl.encodeMidSide(left, right, mid, side);

        REQUIRE_THAT(side, WithinAbs(0.0, 0.0001));
        REQUIRE_THAT(mid, WithinAbs(0.5, 0.0001));
    }

    SECTION("Hard-panned left: equal mid and side") {
        float left = 1.0f;
        float right = 0.0f;
        float mid, side;
        sl.encodeMidSide(left, right, mid, side);

        REQUIRE_THAT(mid, WithinAbs(0.5, 0.0001));
        REQUIRE_THAT(side, WithinAbs(0.5, 0.0001));
    }

    SECTION("Hard-panned right: side is negative") {
        float left = 0.0f;
        float right = 1.0f;
        float mid, side;
        sl.encodeMidSide(left, right, mid, side);

        REQUIRE_THAT(mid, WithinAbs(0.5, 0.0001));
        REQUIRE_THAT(side, WithinAbs(-0.5, 0.0001));
    }

    SECTION("Negative values roundtrip") {
        float left = -0.4f;
        float right = 0.6f;
        float mid, side, lOut, rOut;
        sl.encodeMidSide(left, right, mid, side);
        sl.decodeMidSide(mid, side, lOut, rOut);

        REQUIRE_THAT(lOut, WithinAbs(left, 0.0001));
        REQUIRE_THAT(rOut, WithinAbs(right, 0.0001));
    }
}

TEST_CASE("Mid = (L+R)/2, Side = (L-R)/2", "[stereo][ms][formula]") {
    StereoLink sl;

    float left = 0.8f;
    float right = 0.2f;
    float mid, side;
    sl.encodeMidSide(left, right, mid, side);

    float expectedMid  = (left + right) * 0.5f;
    float expectedSide = (left - right) * 0.5f;

    REQUIRE_THAT(mid, WithinAbs(expectedMid, 0.0001));
    REQUIRE_THAT(side, WithinAbs(expectedSide, 0.0001));
}

TEST_CASE("M/S roundtrip with many random-like values", "[stereo][ms][fuzz]") {
    StereoLink sl;

    float testPairs[][2] = {
        {0.0f, 0.0f}, {1.0f, 1.0f}, {-1.0f, -1.0f},
        {1.0f, -1.0f}, {-1.0f, 1.0f}, {0.123f, 0.789f},
        {-0.5f, 0.5f}, {0.999f, 0.001f}
    };

    for (auto& pair : testPairs) {
        float left = pair[0];
        float right = pair[1];

        DYNAMIC_SECTION("L=" << left << " R=" << right) {
            float mid, side, lOut, rOut;
            sl.encodeMidSide(left, right, mid, side);
            sl.decodeMidSide(mid, side, lOut, rOut);

            REQUIRE_THAT(lOut, WithinAbs(left, 0.0001));
            REQUIRE_THAT(rOut, WithinAbs(right, 0.0001));
        }
    }
}

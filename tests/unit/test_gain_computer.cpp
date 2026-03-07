#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <cmath>
#include "GainComputer.h"

using namespace ezsqueeze;
using Catch::Matchers::WithinAbs;

TEST_CASE("Below threshold produces zero gain reduction", "[gaincomputer][basic]") {
    GainComputer gc;
    gc.setThreshold(-20.0f);
    gc.setRatio(4.0f);
    gc.setKneeWidth(0.0f);

    REQUIRE_THAT(gc.computeGainReduction(-30.0f), WithinAbs(0.0, 0.001));
    REQUIRE_THAT(gc.computeGainReduction(-20.0f), WithinAbs(0.0, 0.001));
    REQUIRE_THAT(gc.computeGainReduction(-50.0f), WithinAbs(0.0, 0.001));
}

TEST_CASE("Above threshold: correct gain reduction", "[gaincomputer][basic]") {
    GainComputer gc;
    gc.setThreshold(-20.0f);
    gc.setRatio(4.0f);
    gc.setKneeWidth(0.0f);

    SECTION("-10 dB input, -20 dB threshold, 4:1 ratio -> 7.5 dB GR") {
        float gr = gc.computeGainReduction(-10.0f);
        REQUIRE_THAT(gr, WithinAbs(-7.5, 0.01));
    }

    SECTION("0 dB input, -20 dB threshold, 4:1 ratio -> 15 dB GR") {
        float gr = gc.computeGainReduction(0.0f);
        REQUIRE_THAT(gr, WithinAbs(-15.0, 0.01));
    }

    SECTION("-15 dB input, -20 dB threshold, 4:1 ratio -> 3.75 dB GR") {
        float gr = gc.computeGainReduction(-15.0f);
        REQUIRE_THAT(gr, WithinAbs(-3.75, 0.01));
    }
}

TEST_CASE("Hard knee: sharp breakpoint at threshold", "[gaincomputer][knee]") {
    GainComputer gc;
    gc.setThreshold(-20.0f);
    gc.setRatio(4.0f);
    gc.setKneeWidth(0.0f);

    float justBelow = gc.computeGainReduction(-20.01f);
    float atThresh  = gc.computeGainReduction(-20.0f);
    float justAbove = gc.computeGainReduction(-19.99f);

    REQUIRE_THAT(justBelow, WithinAbs(0.0, 0.01));
    REQUIRE_THAT(atThresh, WithinAbs(0.0, 0.01));
    REQUIRE(justAbove < -0.001f);
}

TEST_CASE("Soft knee: smooth transition around threshold", "[gaincomputer][knee]") {
    GainComputer gc;
    gc.setThreshold(-20.0f);
    gc.setRatio(4.0f);
    gc.setKneeWidth(6.0f);

    SECTION("Below knee region: no reduction") {
        float gr = gc.computeGainReduction(-24.0f);
        REQUIRE_THAT(gr, WithinAbs(0.0, 0.001));
    }

    SECTION("Above knee region: full ratio applied") {
        float grSoft = gc.computeGainReduction(-10.0f);
        gc.setKneeWidth(0.0f);
        float grHard = gc.computeGainReduction(-10.0f);
        REQUIRE_THAT(grSoft, WithinAbs(grHard, 0.1));
    }

    SECTION("At threshold: approximately half the full GR") {
        float grAtThreshold = gc.computeGainReduction(-20.0f);
        gc.setKneeWidth(0.0f);
        float fullGR = gc.computeGainReduction(-20.0f);
        REQUIRE(std::abs(grAtThreshold) > 0.0f);
        REQUIRE(std::abs(grAtThreshold) <= std::abs(fullGR) + 0.5f);
    }

    SECTION("GR increases monotonically through knee") {
        float prev = 0.0f;
        for (float db = -23.0f; db <= -17.0f; db += 0.5f) {
            float gr = gc.computeGainReduction(db);
            REQUIRE(gr <= prev + 0.001f);
            prev = gr;
        }
    }
}

TEST_CASE("Ratio 1:1 produces no reduction", "[gaincomputer][ratio]") {
    GainComputer gc;
    gc.setThreshold(-20.0f);
    gc.setRatio(1.0f);
    gc.setKneeWidth(0.0f);

    REQUIRE_THAT(gc.computeGainReduction(-10.0f), WithinAbs(0.0, 0.001));
    REQUIRE_THAT(gc.computeGainReduction(0.0f), WithinAbs(0.0, 0.001));
    REQUIRE_THAT(gc.computeGainReduction(10.0f), WithinAbs(0.0, 0.001));
}

TEST_CASE("Infinite ratio acts as limiter", "[gaincomputer][ratio]") {
    GainComputer gc;
    gc.setThreshold(-20.0f);
    gc.setRatio(std::numeric_limits<float>::infinity());
    gc.setKneeWidth(0.0f);

    SECTION("Output stays at threshold for any input above") {
        float gr = gc.computeGainReduction(-10.0f);
        float overshoot = -10.0f - (-20.0f);
        REQUIRE_THAT(gr, WithinAbs(-overshoot, 0.01));
    }

    SECTION("0 dB input clamped to threshold") {
        float gr = gc.computeGainReduction(0.0f);
        float overshoot = 0.0f - (-20.0f);
        REQUIRE_THAT(gr, WithinAbs(-overshoot, 0.01));
    }
}

TEST_CASE("Various ratios produce expected GR", "[gaincomputer][ratio]") {
    GainComputer gc;
    gc.setThreshold(-20.0f);
    gc.setKneeWidth(0.0f);
    float inputDB = -10.0f;
    float overshoot = inputDB - (-20.0f);

    SECTION("2:1 ratio") {
        gc.setRatio(2.0f);
        float expected = -(overshoot * (1.0f - 1.0f / 2.0f));
        REQUIRE_THAT(gc.computeGainReduction(inputDB), WithinAbs(expected, 0.01));
    }

    SECTION("8:1 ratio") {
        gc.setRatio(8.0f);
        float expected = -(overshoot * (1.0f - 1.0f / 8.0f));
        REQUIRE_THAT(gc.computeGainReduction(inputDB), WithinAbs(expected, 0.01));
    }

    SECTION("20:1 ratio") {
        gc.setRatio(20.0f);
        float expected = -(overshoot * (1.0f - 1.0f / 20.0f));
        REQUIRE_THAT(gc.computeGainReduction(inputDB), WithinAbs(expected, 0.01));
    }
}

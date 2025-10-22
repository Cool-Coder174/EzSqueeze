#include <catch2/catch_test_macros.hpp>
#include "dsp/GainComputer.h"

using namespace EzSqueeze::DSP;

TEST_CASE("GainComputer basic curves", "[gain]") {
    GainComputer gc;
    gc.setThreshold(-12.0f);
    gc.setRatio(4.0f);

    SECTION("No GR below threshold") {
        float gr = gc.computeGainReduction(-20.0f);
        REQUIRE(std::abs(gr - 0.0f) < 1e-4f);
    }

    SECTION("Hard knee matches formula above threshold") {
        gc.setKneeMode(KneeMode::Hard);
        float gr = gc.computeGainReduction(-6.0f); // 6dB over threshold
        // reduction = overshoot * (1 - 1/ratio) = 6 * (1 - 0.25) = 4.5 dB
        REQUIRE(std::abs(gr - (-4.5f)) < 0.1f);
    }

    SECTION("Soft knee yields less GR near threshold than hard") {
        gc.setKneeMode(KneeMode::Soft);
        float grSoft = gc.computeGainReduction(-11.0f); // within knee
        gc.setKneeMode(KneeMode::Hard);
        float grHard = gc.computeGainReduction(-11.0f);
        REQUIRE(grSoft > grHard); // less negative (i.e., closer to 0)
    }
}

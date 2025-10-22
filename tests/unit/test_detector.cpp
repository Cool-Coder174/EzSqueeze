#include <catch2/catch_test_macros.hpp>
#include "dsp/Detector.h"

using namespace EzSqueeze::DSP;

TEST_CASE("Detector Peak vs RMS basic behavior", "[detector]") {
    Detector d;
    d.prepare(48000.0, 5.0f);

    SECTION("Peak mode produces higher instantaneous dB than RMS on transient") {
        d.setMode(DetectorMode::Peak);
        float peakDb = d.processSample(1.0f);

        d.reset();
        d.setMode(DetectorMode::RMS);
        float rmsDb = d.processSample(1.0f);

        REQUIRE(peakDb >= rmsDb);
        REQUIRE(peakDb <= 0.0f);
    }
}

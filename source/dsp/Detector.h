#pragma once
#include <cstdint>
#include "MathUtils.h"

namespace ezsqueeze::dsp {

enum class DetectorMode {
    Peak,
    RMS
};

class Detector {
public:
    Detector() = default;

    void prepare(double newSampleRate);
    void reset();

    void setMode(DetectorMode newMode);
    DetectorMode getMode() const { return mode; }

    void setRmsWindowMs(float windowMs);

    // Returns linear level (0..1+)
    float processSample(float inputSample);
    // Returns decibel level (dBFS)
    float processSampleDb(float inputSample);

    float getLastLevelLinear() const { return lastLevelLinear; }
    float getLastLevelDb() const { return lastLevelDb; }

private:
    void updateRmsCoeff();

    double sampleRate { 48000.0 };
    DetectorMode mode { DetectorMode::Peak };

    float rmsWindowMs { 5.0f };
    float rmsCoeff { 0.0f };
    float previousRmsSquared { 0.0f };

    float previousPeak { 0.0f };

    float lastLevelLinear { 0.0f };
    float lastLevelDb { -120.0f };
};

} // namespace ezsqueeze::dsp

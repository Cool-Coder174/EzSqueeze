#include "Detector.h"
#include <algorithm>
#include <cmath>

namespace ezsqueeze::dsp {

void Detector::prepare(double newSampleRate) {
    sampleRate = newSampleRate;
    updateRmsCoeff();
    reset();
}

void Detector::reset() {
    previousPeak = 0.0f;
    previousRmsSquared = 0.0f;
    lastLevelLinear = 0.0f;
    lastLevelDb = -120.0f;
}

void Detector::setMode(DetectorMode newMode) {
    mode = newMode;
}

void Detector::setRmsWindowMs(float windowMs) {
    rmsWindowMs = std::max(0.01f, windowMs);
    updateRmsCoeff();
}

float Detector::processSample(float inputSample) {
    const float x = std::abs(inputSample);

    float level = 0.0f;
    if (mode == DetectorMode::Peak) {
        // Short decay to stabilize instantaneous peak between samples
        constexpr float decayCoeff = 0.95f;
        previousPeak = std::max(x, previousPeak * decayCoeff);
        level = previousPeak;
    } else { // RMS
        previousRmsSquared = (1.0f - rmsCoeff) * previousRmsSquared + rmsCoeff * (x * x);
        level = std::sqrt(previousRmsSquared);
    }

    lastLevelLinear = level;
    lastLevelDb = linearToDb(level);
    return level;
}

float Detector::processSampleDb(float inputSample) {
    (void)processSample(inputSample);
    return lastLevelDb;
}

void Detector::updateRmsCoeff() {
    rmsCoeff = timeToCoeffMs(rmsWindowMs, sampleRate);
}

} // namespace ezsqueeze::dsp

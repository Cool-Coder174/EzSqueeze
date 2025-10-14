#pragma once
#include <cmath>
#include <algorithm>

namespace ezsqueeze::dsp {

struct BiquadState {
    float z1 { 0.0f };
    float z2 { 0.0f };
    void reset() { z1 = 0.0f; z2 = 0.0f; }
};

struct BiquadCoeffs {
    float b0 { 1.0f }, b1 { 0.0f }, b2 { 0.0f };
    float a1 { 0.0f }, a2 { 0.0f };
};

class Biquad {
public:
    void setCoefficients(const BiquadCoeffs& c) { coeffs = c; }
    void reset() { state.reset(); }

    float processSample(float x) {
        const float y = coeffs.b0 * x + state.z1;
        state.z1 = coeffs.b1 * x - coeffs.a1 * y + state.z2;
        state.z2 = coeffs.b2 * x - coeffs.a2 * y;
        return y;
    }

private:
    BiquadCoeffs coeffs {};
    BiquadState state {};
};

// RBJ cookbook
inline BiquadCoeffs makeHighPass(float sampleRate, float cutoffHz, float q = 0.7071f) {
    const float w0 = 2.0f * static_cast<float>(M_PI) * cutoffHz / static_cast<float>(sampleRate);
    const float cosw0 = std::cos(w0);
    const float sinw0 = std::sin(w0);
    const float alpha = sinw0 / (2.0f * q);

    const float b0 =  (1.0f + cosw0) * 0.5f;
    const float b1 = -(1.0f + cosw0);
    const float b2 =  (1.0f + cosw0) * 0.5f;
    const float a0 =   1.0f + alpha;
    const float a1 =  -2.0f * cosw0;
    const float a2 =   1.0f - alpha;

    return { b0 / a0, b1 / a0, b2 / a0, a1 / a0, a2 / a0 };
}

inline BiquadCoeffs makeLowPass(float sampleRate, float cutoffHz, float q = 0.7071f) {
    const float w0 = 2.0f * static_cast<float>(M_PI) * cutoffHz / static_cast<float>(sampleRate);
    const float cosw0 = std::cos(w0);
    const float sinw0 = std::sin(w0);
    const float alpha = sinw0 / (2.0f * q);

    const float b0 =  (1.0f - cosw0) * 0.5f;
    const float b1 =   1.0f - cosw0;
    const float b2 =  (1.0f - cosw0) * 0.5f;
    const float a0 =   1.0f + alpha;
    const float a1 =  -2.0f * cosw0;
    const float a2 =   1.0f - alpha;

    return { b0 / a0, b1 / a0, b2 / a0, a1 / a0, a2 / a0 };
}

} // namespace ezsqueeze::dsp

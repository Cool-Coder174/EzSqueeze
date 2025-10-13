#pragma once

#include <cmath>
#include <array>
#include <algorithm>

namespace ezsqueeze::dsp {

class Biquad {
public:
    enum class Type { LowPass, HighPass };

    void prepare(const double newSampleRate) noexcept {
        sampleRate = newSampleRate;
        reset();
    }

    void reset() noexcept {
        x1 = 0.0f; x2 = 0.0f;
        y1 = 0.0f; y2 = 0.0f;
    }

    void makeHighPass(const float frequencyHz, const float q = 0.7071f) noexcept {
        makeFilter(Type::HighPass, frequencyHz, q);
    }

    void makeLowPass(const float frequencyHz, const float q = 0.7071f) noexcept {
        makeFilter(Type::LowPass, frequencyHz, q);
    }

    float processSample(const float input) noexcept {
        const float output = b0 * input + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;
        x2 = x1; x1 = input;
        y2 = y1; y1 = output;
        return output;
    }

private:
    void makeFilter(const Type type, const float frequencyHz, const float q) noexcept {
        const float pi = 3.14159265358979323846f;
        const float omega = 2.0f * pi * (frequencyHz / static_cast<float>(sampleRate));
        const float sinOmega = std::sin(omega);
        const float cosOmega = std::cos(omega);
        const float alpha = sinOmega / (2.0f * q);

        float b0n = 0.0f, b1n = 0.0f, b2n = 0.0f; // numerator
        float a0n = 1.0f, a1n = 0.0f, a2n = 0.0f; // denominator

        switch (type) {
            case Type::HighPass: {
                b0n =  (1.0f + cosOmega) * 0.5f;
                b1n = -(1.0f + cosOmega);
                b2n =  (1.0f + cosOmega) * 0.5f;
                a0n =  1.0f + alpha;
                a1n = -2.0f * cosOmega;
                a2n =  1.0f - alpha;
                break;
            }
            case Type::LowPass: {
                b0n =  (1.0f - cosOmega) * 0.5f;
                b1n =   1.0f - cosOmega;
                b2n =  (1.0f - cosOmega) * 0.5f;
                a0n =   1.0f + alpha;
                a1n =  -2.0f * cosOmega;
                a2n =   1.0f - alpha;
                break;
            }
        }

        const float a0Inv = 1.0f / a0n;
        b0 = b0n * a0Inv; b1 = b1n * a0Inv; b2 = b2n * a0Inv;
        a1 = a1n * a0Inv; a2 = a2n * a0Inv;

        // reset state to avoid pops with drastic param changes
        x1 = x2 = y1 = y2 = 0.0f;
    }

    double sampleRate { 44100.0 };
    float b0 { 1.0f }, b1 { 0.0f }, b2 { 0.0f };
    float a1 { 0.0f }, a2 { 0.0f };
    float x1 { 0.0f }, x2 { 0.0f };
    float y1 { 0.0f }, y2 { 0.0f };
};

} // namespace ezsqueeze::dsp

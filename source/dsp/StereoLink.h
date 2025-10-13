#pragma once

#include <algorithm>
#include <cmath>

namespace ezsqueeze::dsp {

class StereoLinker {
public:
    void setLinkAmount(const float amount) noexcept {
        linkAmount = std::clamp(amount, 0.0f, 1.0f);
    }

    // Blend independent L/R detector levels with a linked level (max or RMS sum)
    void process(const float leftDetectorLevel,
                 const float rightDetectorLevel,
                 float &outLeftControl,
                 float &outRightControl) const noexcept {
        const float linkedLevel = std::max(leftDetectorLevel, rightDetectorLevel);
        outLeftControl = lerp(leftDetectorLevel, linkedLevel, linkAmount);
        outRightControl = lerp(rightDetectorLevel, linkedLevel, linkAmount);
    }

private:
    static inline float lerp(const float a, const float b, const float t) noexcept {
        return a + (b - a) * std::clamp(t, 0.0f, 1.0f);
    }

    float linkAmount { 1.0f };
};

// Utility encode/decode for Mid/Side
struct MidSide {
    static inline void encode(const float left, const float right, float &mid, float &side) noexcept {
        mid = 0.5f * (left + right);
        side = 0.5f * (left - right);
    }

    static inline void decode(const float mid, const float side, float &left, float &right) noexcept {
        left = mid + side;
        right = mid - side;
    }
};

} // namespace ezsqueeze::dsp

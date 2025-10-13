#pragma once
#include <algorithm>

namespace ezsqueeze::dsp {

// Returns blended control signals for L/R given independent levels and link amount [0..1]
inline void computeStereoLink(float leftLevel,
                              float rightLevel,
                              float linkAmount,
                              float& outLeftControl,
                              float& outRightControl) {
    linkAmount = std::clamp(linkAmount, 0.0f, 1.0f);
    const float linked = std::max(leftLevel, rightLevel); // alternative: sqrt(L^2 + R^2)
    outLeftControl  = (1.0f - linkAmount) * leftLevel  + linkAmount * linked;
    outRightControl = (1.0f - linkAmount) * rightLevel + linkAmount * linked;
}

inline void encodeMS(float left, float right, float& mid, float& side) {
    mid = 0.5f * (left + right);
    side = 0.5f * (left - right);
}

inline void decodeMS(float mid, float side, float& left, float& right) {
    left = mid + side;
    right = mid - side;
}

} // namespace ezsqueeze::dsp

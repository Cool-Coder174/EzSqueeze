#pragma once

#include <algorithm>

namespace ezsqueeze::dsp {

class ParallelMixer {
public:
    void setMix(const float wetAmount) noexcept { mix = std::clamp(wetAmount, 0.0f, 1.0f); }

    float processSample(const float dry, const float wet) const noexcept {
        const float dryAmount = 1.0f - mix;
        return dryAmount * dry + mix * wet;
    }

private:
    float mix { 1.0f }; // 1.0 = fully wet
};

} // namespace ezsqueeze::dsp

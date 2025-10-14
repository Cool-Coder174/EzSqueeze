#pragma once
#include "MathUtils.h"

namespace ezsqueeze::dsp {

enum class KneeType { Hard, Medium, Soft };

class GainComputer {
public:
    void setThresholdDb(float db) { thresholdDb = db; }
    void setRatio(float newRatio) { ratio = newRatio < 1.0f ? 1.0f : newRatio; }

    void setKneeType(KneeType type);
    void setKneeWidthDb(float widthDb) { kneeWidthDb = widthDb < 0.0f ? 0.0f : widthDb; }

    float getThresholdDb() const { return thresholdDb; }
    float getRatio() const { return ratio; }
    float getKneeWidthDb() const { return kneeWidthDb; }
    KneeType getKneeType() const { return kneeType; }

    // Returns negative dB value (<= 0). 0.0 when below knee region
    float computeGainReductionDb(float inputDb) const;

    // Returns multiplicative gain (<= 1.0)
    float computeGainLinear(float inputDb) const { return dbToLinear(computeGainReductionDb(inputDb)); }

private:
    float thresholdDb { -18.0f };
    float ratio { 4.0f };
    KneeType kneeType { KneeType::Hard };
    float kneeWidthDb { 0.0f };
};

} // namespace ezsqueeze::dsp

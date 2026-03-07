#pragma once

#include <cmath>
#include <algorithm>

namespace ezsqueeze
{

/**
 * @brief Peak and RMS level detector for dynamics processing.
 *
 * Measures instantaneous signal level using either peak detection
 * (instant attack, exponential decay) or RMS detection (1-pole IIR
 * running average).
 */
class Detector
{
public:
    /** Detection mode. */
    enum class Mode
    {
        Peak, ///< Instant attack, exponential decay
        RMS   ///< Running mean-square with 1-pole IIR
    };

    /** @brief Set the sample rate for coefficient calculations. */
    void setSampleRate(double sr)
    {
        sampleRate_ = sr;
        recalcCoeffs();
    }

    /** @brief Select peak or RMS detection mode. */
    void setMode(Mode m)
    {
        mode_ = m;
    }

    /**
     * @brief Set the detector window / decay time.
     * @param ms  Window length in milliseconds. Controls decay time
     *            for peak mode and averaging window for RMS mode.
     */
    void setWindowSize(float ms)
    {
        windowMs_ = std::max(ms, 0.1f);
        recalcCoeffs();
    }

    /**
     * @brief Process a single sample and return the detected level.
     * @param sample  Input sample (mono).
     * @return Detected level (linear).
     */
    float process(float sample)
    {
        if (mode_ == Mode::Peak)
        {
            float rectified = std::fabs(sample);
            if (rectified > level_)
            {
                level_ = rectified;
            }
            else
            {
                level_ += decayCoeff_ * (rectified - level_);
            }
        }
        else
        {
            float sq = sample * sample;
            meanSquare_ += rmsCoeff_ * (sq - meanSquare_);
            level_ = std::sqrt(std::max(meanSquare_, 0.0f));
        }

        return level_;
    }

    /**
     * @brief Return the current level in dB with a -120 dB floor.
     * @return Level in dBFS.
     */
    float getLevelDB() const
    {
        return 20.0f * std::log10(std::max(level_, DB_FLOOR_LINEAR));
    }

    /** @brief Reset all internal state to zero. */
    void reset()
    {
        level_ = 0.0f;
        meanSquare_ = 0.0f;
    }

private:
    static constexpr float DB_FLOOR_LINEAR = 1e-6f; // -120 dBFS

    void recalcCoeffs()
    {
        if (sampleRate_ <= 0.0)
            return;

        float timeSec = windowMs_ * 0.001f;
        float coeff = 1.0f - std::exp(-1.0f / static_cast<float>(timeSec * sampleRate_));
        decayCoeff_ = coeff;
        rmsCoeff_ = coeff;
    }

    Mode mode_ = Mode::Peak;
    double sampleRate_ = 44100.0;
    float windowMs_ = 50.0f;
    float level_ = 0.0f;
    float meanSquare_ = 0.0f;
    float decayCoeff_ = 0.01f;
    float rmsCoeff_ = 0.01f;
};

} // namespace ezsqueeze

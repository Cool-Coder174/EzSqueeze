#pragma once

#include <array>
#include <cmath>

namespace EzSqueeze::DSP {

/**
 * @brief Sidechain filter chain for HPF and LPF filtering
 * 
 * Implements Butterworth 2nd-order high-pass and low-pass filters
 * for sidechain signal conditioning. All processing is real-time safe.
 */
class SidechainFilter
{
public:
    /**
     * @brief Filter types
     */
    enum class FilterType
    {
        HighPass,  ///< High-pass filter
        LowPass    ///< Low-pass filter
    };

    /**
     * @brief Constructor
     * @param sampleRate Sample rate in Hz
     * @param hpfFreq High-pass filter frequency in Hz (0 = disabled)
     * @param lpfFreq Low-pass filter frequency in Hz (0 = disabled)
     */
    SidechainFilter(float sampleRate = 44100.0f, float hpfFreq = 0.0f, float lpfFreq = 0.0f);

    /**
     * @brief Prepare the filter chain
     * @param sampleRate New sample rate
     * @param hpfFreq High-pass filter frequency in Hz (0 = disabled)
     * @param lpfFreq Low-pass filter frequency in Hz (0 = disabled)
     */
    void prepare(float sampleRate, float hpfFreq = 0.0f, float lpfFreq = 0.0f);

    /**
     * @brief Process a single sample
     * @param input Input sample
     * @return Filtered output sample
     */
    float processSample(float input);

    /**
     * @brief Process a block of samples
     * @param input Input buffer
     * @param output Output buffer
     * @param numSamples Number of samples to process
     */
    void processBlock(const float* input, float* output, int numSamples);

    /**
     * @brief Reset the filter state
     */
    void reset();

    /**
     * @brief Set high-pass filter frequency
     * @param freq Frequency in Hz (0 = disabled)
     */
    void setHPFFrequency(float freq);

    /**
     * @brief Set low-pass filter frequency
     * @param freq Frequency in Hz (0 = disabled)
     */
    void setLPFFrequency(float freq);

    /**
     * @brief Get current HPF frequency
     * @return Frequency in Hz (0 = disabled)
     */
    float getHPFFrequency() const { return hpfFreq_; }

    /**
     * @brief Get current LPF frequency
     * @return Frequency in Hz (0 = disabled)
     */
    float getLPFFrequency() const { return lpfFreq_; }

    /**
     * @brief Check if HPF is enabled
     * @return True if HPF is enabled
     */
    bool isHPFEnabled() const { return hpfFreq_ > 0.0f; }

    /**
     * @brief Check if LPF is enabled
     * @return True if LPF is enabled
     */
    bool isLPFEnabled() const { return lpfFreq_ > 0.0f; }

private:
    struct BiquadCoefficients
    {
        float b0, b1, b2;  // Feedforward coefficients
        float a1, a2;      // Feedback coefficients
    };

    struct BiquadState
    {
        float x1, x2;      // Input history
        float y1, y2;      // Output history
    };

    float sampleRate_;
    float hpfFreq_;
    float lpfFreq_;
    
    BiquadCoefficients hpfCoeffs_;
    BiquadCoefficients lpfCoeffs_;
    BiquadState hpfState_;
    BiquadState lpfState_;

    /**
     * @brief Calculate Butterworth 2nd-order coefficients
     * @param freq Cutoff frequency in Hz
     * @param type Filter type (HPF or LPF)
     * @return Biquad coefficients
     */
    BiquadCoefficients calculateButterworthCoeffs(float freq, FilterType type) const;

    /**
     * @brief Process a single biquad filter
     * @param input Input sample
     * @param coeffs Filter coefficients
     * @param state Filter state
     * @return Filtered output sample
     */
    float processBiquad(float input, const BiquadCoefficients& coeffs, BiquadState& state);

    /**
     * @brief Reset biquad state
     * @param state State to reset
     */
    void resetBiquadState(BiquadState& state);

    /**
     * @brief Clamp frequency to valid range
     * @param freq Frequency in Hz
     * @return Clamped frequency
     */
    float clampFrequency(float freq) const;
};

} // namespace EzSqueeze::DSP
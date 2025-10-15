/**
 * @file SidechainFilter.h
 * @brief Sidechain filtering for frequency-selective compression
 * 
 * HPF/LPF filters for sidechain signal to focus compression
 * on specific frequency ranges or remove problematic frequencies.
 * 
 * @author Isaac Hernandez
 * @date October 2025
 */

#pragma once

#include <cmath>
#include <algorithm>

namespace EzSqueeze {
namespace DSP {

/**
 * @class BiquadFilter
 * @brief Second-order IIR filter (biquad)
 * 
 * Standard biquad filter implementation using direct form I.
 * Used for building HPF and LPF filters.
 * 
 * RT-Safe: Yes (no allocations)
 * Complexity: O(1) per sample
 */
class BiquadFilter
{
public:
    BiquadFilter() = default;
    ~BiquadFilter() = default;

    /**
     * @brief Configure as highpass filter
     * @param sampleRate Sample rate in Hz
     * @param frequency Cutoff frequency in Hz
     * @param Q Quality factor (0.707 for Butterworth)
     */
    void makeHighPass(double sampleRate, float frequency, float Q = 0.707f);

    /**
     * @brief Configure as lowpass filter
     * @param sampleRate Sample rate in Hz
     * @param frequency Cutoff frequency in Hz
     * @param Q Quality factor (0.707 for Butterworth)
     */
    void makeLowPass(double sampleRate, float frequency, float Q = 0.707f);

    /**
     * @brief Process single sample
     * @param input Input sample
     * @return Filtered output sample
     */
    float processSample(float input);

    /**
     * @brief Reset filter state
     */
    void reset();

private:
    // Biquad coefficients
    float b0 = 1.0f, b1 = 0.0f, b2 = 0.0f;
    float a1 = 0.0f, a2 = 0.0f;
    
    // State variables
    float x1 = 0.0f, x2 = 0.0f;  // Input history
    float y1 = 0.0f, y2 = 0.0f;  // Output history
    
    /**
     * @brief Calculate biquad coefficients
     * @param sampleRate Sample rate
     * @param frequency Center/cutoff frequency
     * @param Q Quality factor
     * @param isHighPass True for HPF, false for LPF
     */
    void calculateCoefficients(double sampleRate, float frequency, 
                              float Q, bool isHighPass);
};

/**
 * @class SidechainFilter
 * @brief HPF and LPF chain for sidechain processing
 * 
 * Combines highpass and lowpass filters to shape the
 * frequency response of the sidechain detection signal.
 * 
 * RT-Safe: Yes (no allocations)
 * Complexity: O(1) per sample
 */
class SidechainFilter
{
public:
    SidechainFilter() = default;
    ~SidechainFilter() = default;

    /**
     * @brief Prepare filters
     * @param sampleRate Sample rate in Hz
     */
    void prepare(double sampleRate);

    /**
     * @brief Set highpass filter frequency
     * @param frequency HPF frequency in Hz (20-400Hz typical)
     */
    void setHighPassFrequency(float frequency);

    /**
     * @brief Set lowpass filter frequency
     * @param frequency LPF frequency in Hz (4-16kHz typical)
     */
    void setLowPassFrequency(float frequency);

    /**
     * @brief Enable/disable highpass filter
     * @param enabled True to enable HPF
     */
    void setHighPassEnabled(bool enabled);

    /**
     * @brief Enable/disable lowpass filter
     * @param enabled True to enable LPF
     */
    void setLowPassEnabled(bool enabled);

    /**
     * @brief Process sample through filter chain
     * @param input Input sample
     * @return Filtered output sample
     */
    float processSample(float input);

    /**
     * @brief Reset all filters
     */
    void reset();

private:
    BiquadFilter m_highPass;
    BiquadFilter m_lowPass;
    
    double m_sampleRate = 48000.0;
    float m_hpfFreq = 20.0f;
    float m_lpfFreq = 16000.0f;
    
    bool m_hpfEnabled = false;
    bool m_lpfEnabled = false;
};

} // namespace DSP
} // namespace EzSqueeze

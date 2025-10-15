/**
 * @file Detector.h
 * @brief Level detection engine for EzSqueeze compressor
 * 
 * Provides Peak and RMS detection modes for envelope following.
 * Implements real-time safe level detection with configurable
 * time constants and conversion to dB scale.
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
 * @brief Detection mode enumeration
 */
enum class DetectorMode
{
    Peak,  ///< Instantaneous peak detection
    RMS    ///< Root-mean-square average detection
};

/**
 * @class Detector
 * @brief Level detection with Peak/RMS modes
 * 
 * The Detector class converts audio signals into control signals
 * for dynamic range compression. It supports both peak and RMS
 * detection modes with configurable window sizes for RMS.
 * 
 * RT-Safe: Yes (no allocations after prepare())
 * Complexity: O(1) per sample
 */
class Detector
{
public:
    Detector() = default;
    ~Detector() = default;

    /**
     * @brief Prepare detector for processing
     * @param sampleRate Sample rate in Hz
     * @param rmsWindowMs RMS averaging window in milliseconds (1-10ms typical)
     */
    void prepare(double sampleRate, float rmsWindowMs = 5.0f);

    /**
     * @brief Set detection mode
     * @param mode Peak or RMS
     */
    void setMode(DetectorMode mode);

    /**
     * @brief Process single sample and return level in dB
     * @param input Audio sample (linear amplitude)
     * @return Level in dBFS (-120 to 0)
     */
    float processSample(float input);

    /**
     * @brief Reset detector state
     */
    void reset();

private:
    DetectorMode m_mode = DetectorMode::Peak;
    double m_sampleRate = 48000.0;
    
    // Peak detection state
    float m_peakLevel = 0.0f;
    float m_peakDecay = 0.999f;
    
    // RMS detection state
    float m_rmsSquared = 0.0f;
    float m_rmsAlpha = 0.0f;  // Smoothing coefficient
    
    // Constants
    static constexpr float MIN_LEVEL_DB = -120.0f;
    static constexpr float MIN_LINEAR = 1e-6f;  // Avoid log(0)
    
    /**
     * @brief Convert linear amplitude to dB
     * @param linear Linear amplitude value
     * @return Level in dB
     */
    inline float linearToDb(float linear) const
    {
        return 20.0f * std::log10(std::max(linear, MIN_LINEAR));
    }
    
    /**
     * @brief Calculate RMS smoothing coefficient from window time
     * @param windowMs Window size in milliseconds
     * @return Alpha coefficient for exponential averaging
     */
    float calculateRmsAlpha(float windowMs);
};

} // namespace DSP
} // namespace EzSqueeze

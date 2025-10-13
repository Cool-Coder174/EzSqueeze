#pragma once

#include <cmath>
#include <algorithm>

namespace EzSqueeze::DSP {

/**
 * @brief Detector engine for converting audio signals to control signals
 * 
 * Supports both Peak and RMS detection modes with configurable parameters.
 * All processing is real-time safe with no dynamic allocations.
 */
class DetectorEngine
{
public:
    enum class Mode
    {
        Peak,   ///< Instantaneous peak detection
        RMS     ///< Root-mean-square average level detection
    };

    /**
     * @brief Constructor
     * @param sampleRate Sample rate in Hz
     * @param mode Detection mode (Peak or RMS)
     * @param rmsWindowMs RMS window size in milliseconds (only used for RMS mode)
     */
    DetectorEngine(float sampleRate = 44100.0f, Mode mode = Mode::Peak, float rmsWindowMs = 5.0f);

    /**
     * @brief Prepare the detector for processing
     * @param sampleRate New sample rate
     * @param mode Detection mode
     * @param rmsWindowMs RMS window size in milliseconds
     */
    void prepare(float sampleRate, Mode mode, float rmsWindowMs = 5.0f);

    /**
     * @brief Process a single sample
     * @param input Input sample
     * @return Detected level in dBFS
     */
    float processSample(float input);

    /**
     * @brief Process a block of samples
     * @param input Input buffer
     * @param output Output buffer (detected levels in dBFS)
     * @param numSamples Number of samples to process
     */
    void processBlock(const float* input, float* output, int numSamples);

    /**
     * @brief Reset the detector state
     */
    void reset();

    /**
     * @brief Get the current detection mode
     * @return Current mode
     */
    Mode getMode() const { return mode_; }

    /**
     * @brief Get the RMS window size in samples
     * @return Window size in samples
     */
    int getRMSWindowSamples() const { return rmsWindowSamples_; }

    /**
     * @brief Get the current RMS level (only valid in RMS mode)
     * @return Current RMS level in linear units
     */
    float getCurrentRMSLevel() const { return rmsLevel_; }

    /**
     * @brief Get the current peak level (only valid in Peak mode)
     * @return Current peak level in linear units
     */
    float getCurrentPeakLevel() const { return peakLevel_; }

private:
    Mode mode_;
    float sampleRate_;
    int rmsWindowSamples_;
    
    // Peak detection state
    float peakLevel_;
    float peakDecayCoeff_;
    
    // RMS detection state
    float rmsLevel_;
    float rmsSquared_;
    float rmsAlpha_;
    
    // Constants
    static constexpr float MIN_LEVEL = 1e-6f;  // -120 dBFS floor
    static constexpr float PEAK_DECAY_RATE = 0.999f;  // Peak decay coefficient
    static constexpr float DB_CONVERSION = 20.0f;  // 20 * log10 for dB conversion

    /**
     * @brief Convert linear level to dBFS
     * @param linearLevel Linear level (0.0 to 1.0)
     * @return Level in dBFS
     */
    float linearToDB(float linearLevel) const;

    /**
     * @brief Update RMS calculation
     * @param input Input sample
     */
    void updateRMS(float input);

    /**
     * @brief Update peak calculation
     * @param input Input sample
     */
    void updatePeak(float input);
};

} // namespace EzSqueeze::DSP
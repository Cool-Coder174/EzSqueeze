#pragma once

#include <juce_dsp/juce_dsp.h>
#include <memory>
#include <cassert>

namespace ezsqueeze
{

/**
 * @brief Wrapper around juce::dsp::Oversampling for transparent up/downsampling.
 *
 * Manages a JUCE oversampling object at 1×, 2×, 4× or 8× factors.
 * All heap allocation happens in prepare(); the process path
 * (upsample / downsample) is real-time safe.
 */
class Oversampling
{
public:
    /** Oversampling factor. */
    enum class Factor
    {
        x1 = 0,
        x2 = 1,
        x4 = 2,
        x8 = 3
    };

    /**
     * @brief Set the desired oversampling factor.
     *
     * Must be called before prepare(). Changing it after prepare()
     * requires a new call to prepare().
     *
     * @param f  Oversampling factor.
     */
    void setFactor(Factor f)
    {
        factor_ = f;
    }

    /**
     * @brief Allocate internal buffers. NOT real-time safe.
     * @param sampleRate  Host sample rate.
     * @param blockSize   Maximum expected block size.
     */
    void prepare(double sampleRate, int blockSize)
    {
        sampleRate_ = sampleRate;
        int order = static_cast<int>(factor_);

        if (order == 0)
        {
            oversampler_.reset();
            return;
        }

        oversampler_ = std::make_unique<juce::dsp::Oversampling<float>>(
            2, // stereo
            order,
            juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR,
            true  // max quality
        );

        oversampler_->initProcessing(static_cast<size_t>(blockSize));
    }

    /**
     * @brief Upsample an audio block.
     * @param block  Input block at native sample rate.
     * @return Oversampled audio block (owned by the oversampler).
     */
    juce::dsp::AudioBlock<float> upsample(juce::dsp::AudioBlock<float>& block)
    {
        if (!oversampler_)
            return block;
        return oversampler_->processSamplesUp(block);
    }

    /**
     * @brief Downsample back to the original rate, writing into the original block.
     * @param block  The original (non-oversampled) block to write results into.
     */
    void downsample(juce::dsp::AudioBlock<float>& block)
    {
        if (!oversampler_)
            return;
        oversampler_->processSamplesDown(block);
    }

    /**
     * @brief Get the oversampling latency in samples (at the native rate).
     * @return Latency in samples.
     */
    int getLatencySamples() const
    {
        if (!oversampler_)
            return 0;
        return static_cast<int>(oversampler_->getLatencyInSamples());
    }

    /**
     * @brief Return the CPU cost multiplier (1, 2, 4, or 8).
     * @return Multiplier relative to 1× processing.
     */
    float getCPUMultiplier() const
    {
        return static_cast<float>(1 << static_cast<int>(factor_));
    }

    /** @brief Get the effective oversampled sample rate. */
    double getOversampledRate() const
    {
        return sampleRate_ * static_cast<double>(1 << static_cast<int>(factor_));
    }

private:
    Factor factor_ = Factor::x1;
    double sampleRate_ = 44100.0;
    std::unique_ptr<juce::dsp::Oversampling<float>> oversampler_;
};

} // namespace ezsqueeze

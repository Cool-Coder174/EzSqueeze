/**
 * @file PluginProcessor.h
 * @brief Main audio processor for the EzSqueeze dynamics compressor.
 *
 * @copyright 2026 Isaac Hernandez. Licensed under GPL-3.0.
 */

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

#include "../dsp/AutoMakeup.h"
#include "../dsp/Detector.h"
#include "../dsp/EnvelopeFollower.h"
#include "../dsp/GainComputer.h"
#include "../dsp/LookaheadBuffer.h"
#include "../dsp/Oversampling.h"
#include "../dsp/ProgramDependentRelease.h"

#include "../dsp/SidechainFilter.h"
#include "../dsp/StereoLink.h"
#include "../dsp/VibeWheel.h"
#include "../../modules/CloudGainPreamp.h"
#include "../../modules/DualStageCompressor.h"
#include "../../modules/TransientSculptor.h"
#include "Parameters.h"

/**
 * @class EzSqueezeProcessor
 * @brief Core audio processor implementing the full EzSqueeze signal chain.
 *
 * Owns all DSP objects, manages parameter synchronisation via APVTS,
 * and reports latency from lookahead + oversampling.
 */
class EzSqueezeProcessor : public juce::AudioProcessor
{
public:
    EzSqueezeProcessor();
    ~EzSqueezeProcessor() override = default;

    // --- AudioProcessor overrides ---

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>& buffer,
                      juce::MidiBuffer& midiMessages) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    /** @brief Total plugin latency in samples (lookahead + oversampling). */
    int getLatencySamples() const;

    juce::AudioProcessorValueTreeState apvts;

private:
    // --- DSP Modules ---

    ezsqueeze::Detector detectorL_;
    ezsqueeze::Detector detectorR_;
    ezsqueeze::GainComputer gainComputer_;
    ezsqueeze::EnvelopeFollower envelopeFollowerL_;
    ezsqueeze::EnvelopeFollower envelopeFollowerR_;
    ezsqueeze::LookaheadBuffer lookaheadBufferL_;
    ezsqueeze::LookaheadBuffer lookaheadBufferR_;
    ezsqueeze::StereoLink stereoLink_;
    ezsqueeze::SidechainFilter sidechainFilterL_;
    ezsqueeze::SidechainFilter sidechainFilterR_;
    ezsqueeze::AutoMakeup autoMakeup_;
    ezsqueeze::ProgramDependentRelease programDependentRelease_;
    ezsqueeze::Oversampling oversampling_;
    ezsqueeze::VibeWheel vibeWheelL_;
    ezsqueeze::VibeWheel vibeWheelR_;
    ezsqueeze::DualStageCompressor dualStageCompressor_;
    ezsqueeze::CloudGainPreamp cloudGainPreampL_;
    ezsqueeze::CloudGainPreamp cloudGainPreampR_;
    ezsqueeze::TransientSculptor transientSculptor_;

    double currentSampleRate_ = 44100.0;
    int currentBlockSize_     = 512;
    float prevLevel_          = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EzSqueezeProcessor)
};

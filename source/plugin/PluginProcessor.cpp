/**
 * @file PluginProcessor.cpp
 * @brief Implementation of the EzSqueeze audio processor and signal chain.
 *
 * @copyright 2026 Isaac Hernandez. Licensed under GPL-3.0.
 */

#include "PluginProcessor.h"

#include "PluginEditor.h"

//==============================================================================

EzSqueezeProcessor::EzSqueezeProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "PARAMETERS", ezsqueeze::createParameterLayout())
{
}

//==============================================================================

void EzSqueezeProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate_ = sampleRate;
    currentBlockSize_  = samplesPerBlock;

    detector_.prepare(sampleRate, samplesPerBlock);
    gainComputer_.prepare(sampleRate, samplesPerBlock);
    envelopeFollower_.prepare(sampleRate, samplesPerBlock);
    lookaheadBufferL_.prepare(sampleRate, samplesPerBlock);
    lookaheadBufferR_.prepare(sampleRate, samplesPerBlock);
    stereoLink_.prepare(sampleRate, samplesPerBlock);
    sidechainFilter_.prepare(sampleRate, samplesPerBlock);
    autoMakeup_.prepare(sampleRate, samplesPerBlock);
    programDependentRelease_.prepare(sampleRate, samplesPerBlock);
    oversampling_.prepare(sampleRate, samplesPerBlock);
    saturation_.prepare(sampleRate, samplesPerBlock);
    vibeWheel_.prepare(sampleRate, samplesPerBlock);
    dualStageCompressor_.prepare(sampleRate, samplesPerBlock);
    cloudGainPreamp_.prepare(sampleRate, samplesPerBlock);
    transientSculptor_.prepare(sampleRate, samplesPerBlock);

    setLatencySamples(getLatencySamples());
}

void EzSqueezeProcessor::releaseResources()
{
    detector_.reset();
    gainComputer_.reset();
    envelopeFollower_.reset();
    lookaheadBufferL_.reset();
    lookaheadBufferR_.reset();
    stereoLink_.reset();
    sidechainFilter_.reset();
    autoMakeup_.reset();
    programDependentRelease_.reset();
    oversampling_.reset();
    saturation_.reset();
    vibeWheel_.reset();
    dualStageCompressor_.reset();
    cloudGainPreamp_.reset();
    transientSculptor_.reset();
}

//==============================================================================

void EzSqueezeProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                      juce::MidiBuffer& /*midiMessages*/)
{
    juce::ScopedNoDenormals noDenormals;

    const int numChannels = buffer.getNumChannels();
    const int numSamples  = buffer.getNumSamples();

    if (numChannels < 2 || numSamples == 0)
        return;

    // ---- 1. Read all parameters from APVTS ----

    const float thresholdDb  = apvts.getRawParameterValue(ezsqueeze::ParamID::threshold)->load();
    const float ratioVal     = apvts.getRawParameterValue(ezsqueeze::ParamID::ratio)->load();
    const float attackMs     = apvts.getRawParameterValue(ezsqueeze::ParamID::attack)->load();
    const float releaseMs    = apvts.getRawParameterValue(ezsqueeze::ParamID::release)->load();
    const int   kneeIndex    = static_cast<int>(
        apvts.getRawParameterValue(ezsqueeze::ParamID::knee)->load());
    const float makeupDb     = apvts.getRawParameterValue(ezsqueeze::ParamID::makeup)->load();
    const float mixPct       = apvts.getRawParameterValue(ezsqueeze::ParamID::mix)->load();
    const int   detectorMode = static_cast<int>(
        apvts.getRawParameterValue(ezsqueeze::ParamID::detector)->load());
    const float lookaheadMs  = apvts.getRawParameterValue(ezsqueeze::ParamID::lookahead)->load();
    const float stereoLinkPct =
        apvts.getRawParameterValue(ezsqueeze::ParamID::stereoLink)->load();
    const bool msMode =
        apvts.getRawParameterValue(ezsqueeze::ParamID::msMode)->load() > 0.5f;
    const float scHPFFreq    = apvts.getRawParameterValue(ezsqueeze::ParamID::scHPF)->load();
    const float scLPFFreq    = apvts.getRawParameterValue(ezsqueeze::ParamID::scLPF)->load();
    const int   osIndex      = static_cast<int>(
        apvts.getRawParameterValue(ezsqueeze::ParamID::oversampling)->load());
    const bool ecoMode =
        apvts.getRawParameterValue(ezsqueeze::ParamID::ecoMode)->load() > 0.5f;
    const bool autoReleaseOn =
        apvts.getRawParameterValue(ezsqueeze::ParamID::autoRelease)->load() > 0.5f;
    const bool autoMakeupOn =
        apvts.getRawParameterValue(ezsqueeze::ParamID::autoMakeup)->load() > 0.5f;
    const float vibeAmount   = apvts.getRawParameterValue(ezsqueeze::ParamID::vibeWheel)->load();
    const bool  dualStackOn =
        apvts.getRawParameterValue(ezsqueeze::ParamID::dualStack)->load() > 0.5f;
    const float cloudGainDb  = apvts.getRawParameterValue(ezsqueeze::ParamID::cloudGain)->load();
    const int   impedanceIdx = static_cast<int>(
        apvts.getRawParameterValue(ezsqueeze::ParamID::impedance)->load());
    const float snapPct      = apvts.getRawParameterValue(ezsqueeze::ParamID::snap)->load();
    const float bodyPct      = apvts.getRawParameterValue(ezsqueeze::ParamID::body)->load();
    const float deSnapPct    = apvts.getRawParameterValue(ezsqueeze::ParamID::deSnap)->load();
    const bool  bypassed =
        apvts.getRawParameterValue(ezsqueeze::ParamID::bypass)->load() > 0.5f;

    if (bypassed)
        return;

    // Keep a dry copy for wet/dry mixing
    juce::AudioBuffer<float> dryBuffer;
    const bool needsMix = mixPct < 99.9f;
    if (needsMix)
    {
        dryBuffer.makeCopyOf(buffer);
    }

    // ---- 2. Cloud-Gain preamp ----

    cloudGainPreamp_.process(buffer, cloudGainDb, impedanceIdx);

    // ---- 3. Upsample (if oversampling enabled and not eco mode) ----

    const bool shouldOversample = osIndex > 0 && !ecoMode;
    if (shouldOversample)
    {
        oversampling_.setFactor(osIndex);
        oversampling_.upsample(buffer);
    }

    // ---- 4. Sidechain filter ----

    juce::AudioBuffer<float> scBuffer;
    scBuffer.makeCopyOf(buffer);
    sidechainFilter_.process(scBuffer, scHPFFreq, scLPFFreq);

    // ---- 5. Detect levels (Peak / RMS) ----

    auto envL = detector_.process(scBuffer.getReadPointer(0), numSamples, detectorMode);
    auto envR = detector_.process(scBuffer.getReadPointer(1), numSamples, detectorMode);

    // ---- 6. Stereo link ----

    stereoLink_.process(envL, envR, stereoLinkPct, msMode);

    // ---- 7. Compute gain reduction (with knee) ----

    auto grL = gainComputer_.compute(envL, thresholdDb, ratioVal, kneeIndex);
    auto grR = gainComputer_.compute(envR, thresholdDb, ratioVal, kneeIndex);

    // ---- 8. Apply envelope (attack/release, program-dependent if enabled) ----

    float effectiveReleaseMs = releaseMs;
    if (autoReleaseOn)
    {
        effectiveReleaseMs = programDependentRelease_.compute(grL, grR, releaseMs);
    }

    envelopeFollower_.process(grL, attackMs, effectiveReleaseMs);
    envelopeFollower_.process(grR, attackMs, effectiveReleaseMs);

    // ---- 9. Apply lookahead delay ----

    lookaheadBufferL_.process(buffer.getWritePointer(0), numSamples, lookaheadMs);
    lookaheadBufferR_.process(buffer.getWritePointer(1), numSamples, lookaheadMs);

    // ---- 10. Apply gain reduction ----

    for (int i = 0; i < numSamples; ++i)
    {
        buffer.getWritePointer(0)[i] *= grL;
        buffer.getWritePointer(1)[i] *= grR;
    }

    // ---- 11. Dual-stage compression (if enabled) ----

    if (dualStackOn)
    {
        dualStageCompressor_.process(buffer);
    }

    // ---- 12. Transient sculptor ----

    transientSculptor_.process(buffer, snapPct, bodyPct, deSnapPct);

    // ---- 13. Vibe Wheel ----

    vibeWheel_.process(buffer, vibeAmount);

    // ---- 14. Downsample ----

    if (shouldOversample)
    {
        oversampling_.downsample(buffer);
    }

    // ---- 15. Mix dry/wet ----

    if (needsMix)
    {
        const float wet = mixPct / 100.0f;
        const float dry = 1.0f - wet;

        for (int ch = 0; ch < numChannels; ++ch)
        {
            auto* wetData = buffer.getWritePointer(ch);
            auto* dryData = dryBuffer.getReadPointer(ch);

            for (int i = 0; i < numSamples; ++i)
            {
                wetData[i] = wetData[i] * wet + dryData[i] * dry;
            }
        }
    }

    // ---- 16. Makeup gain (auto or manual) ----

    if (autoMakeupOn)
    {
        const float autoGainDb = autoMakeup_.compute(thresholdDb, ratioVal, kneeIndex);
        const float autoGain   = juce::Decibels::decibelsToGain(autoGainDb);
        buffer.applyGain(autoGain);
    }
    else if (makeupDb > 0.0f)
    {
        buffer.applyGain(juce::Decibels::decibelsToGain(makeupDb));
    }

    setLatencySamples(getLatencySamples());
}

//==============================================================================

int EzSqueezeProcessor::getLatencySamples() const
{
    const float lookaheadMs =
        apvts.getRawParameterValue(ezsqueeze::ParamID::lookahead)->load();
    const int lookaheadSamples =
        static_cast<int>(std::ceil(lookaheadMs * 0.001 * currentSampleRate_));

    const int oversamplingLatency = oversampling_.getLatencySamples();

    return lookaheadSamples + oversamplingLatency;
}

//==============================================================================

juce::AudioProcessorEditor* EzSqueezeProcessor::createEditor()
{
    return new EzSqueezeEditor(*this);
}

//==============================================================================

void EzSqueezeProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    auto xml   = state.createXml();
    copyXmlToBinary(*xml, destData);
}

void EzSqueezeProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    auto xml = getXmlFromBinary(data, sizeInBytes);
    if (xml != nullptr && xml->hasTagName(apvts.state.getType()))
    {
        apvts.replaceState(juce::ValueTree::fromXml(*xml));
    }
}

//==============================================================================

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new EzSqueezeProcessor();
}

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

    int osIdx = static_cast<int>(
        apvts.getRawParameterValue(ezsqueeze::ParamID::oversampling)->load());
    oversampling_.setFactor(static_cast<ezsqueeze::Oversampling::Factor>(osIdx));
    oversampling_.prepare(sampleRate, samplesPerBlock);

    double effectiveRate = oversampling_.getOversampledRate();

    detectorL_.setSampleRate(effectiveRate);
    detectorR_.setSampleRate(effectiveRate);
    envelopeFollowerL_.setSampleRate(effectiveRate);
    envelopeFollowerR_.setSampleRate(effectiveRate);
    sidechainFilterL_.setSampleRate(effectiveRate);
    sidechainFilterR_.setSampleRate(effectiveRate);
    programDependentRelease_.setSampleRate(effectiveRate);
    vibeWheelL_.setSampleRate(effectiveRate);
    vibeWheelR_.setSampleRate(effectiveRate);
    dualStageCompressor_.setSampleRate(effectiveRate);
    transientSculptor_.setSampleRate(effectiveRate);

    cloudGainPreampL_.setSampleRate(sampleRate);
    cloudGainPreampR_.setSampleRate(sampleRate);

    lookaheadBufferL_.reset();
    lookaheadBufferR_.reset();

    setLatencySamples(getLatencySamples());
}

void EzSqueezeProcessor::releaseResources()
{
    detectorL_.reset();
    detectorR_.reset();
    envelopeFollowerL_.reset();
    envelopeFollowerR_.reset();
    lookaheadBufferL_.reset();
    lookaheadBufferR_.reset();
    sidechainFilterL_.reset();
    sidechainFilterR_.reset();
    autoMakeup_.reset();
    programDependentRelease_.reset();
    vibeWheelL_.reset();
    vibeWheelR_.reset();
    dualStageCompressor_.reset();
    cloudGainPreampL_.reset();
    cloudGainPreampR_.reset();
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

    const float thresholdDb = apvts.getRawParameterValue(ezsqueeze::ParamID::threshold)->load();
    const float ratioVal    = apvts.getRawParameterValue(ezsqueeze::ParamID::ratio)->load();
    const float attackMs    = apvts.getRawParameterValue(ezsqueeze::ParamID::attack)->load();
    const float releaseMs   = apvts.getRawParameterValue(ezsqueeze::ParamID::release)->load();
    const int   kneeIndex   = static_cast<int>(
        apvts.getRawParameterValue(ezsqueeze::ParamID::knee)->load());
    const float makeupDb    = apvts.getRawParameterValue(ezsqueeze::ParamID::makeup)->load();
    const float mixPct      = apvts.getRawParameterValue(ezsqueeze::ParamID::mix)->load();
    const int   detectorMode = static_cast<int>(
        apvts.getRawParameterValue(ezsqueeze::ParamID::detector)->load());
    const float lookaheadMs = apvts.getRawParameterValue(ezsqueeze::ParamID::lookahead)->load();
    const float stereoLinkPct =
        apvts.getRawParameterValue(ezsqueeze::ParamID::stereoLink)->load();
    const bool msMode =
        apvts.getRawParameterValue(ezsqueeze::ParamID::msMode)->load() > 0.5f;
    const float scHPFFreq   = apvts.getRawParameterValue(ezsqueeze::ParamID::scHPF)->load();
    const float scLPFFreq   = apvts.getRawParameterValue(ezsqueeze::ParamID::scLPF)->load();
    const int   osIndex     = static_cast<int>(
        apvts.getRawParameterValue(ezsqueeze::ParamID::oversampling)->load());
    const bool ecoMode =
        apvts.getRawParameterValue(ezsqueeze::ParamID::ecoMode)->load() > 0.5f;
    const bool autoReleaseOn =
        apvts.getRawParameterValue(ezsqueeze::ParamID::autoRelease)->load() > 0.5f;
    const bool autoMakeupOn =
        apvts.getRawParameterValue(ezsqueeze::ParamID::autoMakeup)->load() > 0.5f;
    const float vibeAmount  = apvts.getRawParameterValue(ezsqueeze::ParamID::vibeWheel)->load();
    const bool  dualStackOn =
        apvts.getRawParameterValue(ezsqueeze::ParamID::dualStack)->load() > 0.5f;
    const float cloudGainDb = apvts.getRawParameterValue(ezsqueeze::ParamID::cloudGain)->load();
    const int   impedanceIdx = static_cast<int>(
        apvts.getRawParameterValue(ezsqueeze::ParamID::impedance)->load());
    const float snapPct     = apvts.getRawParameterValue(ezsqueeze::ParamID::snap)->load();
    const float bodyPct     = apvts.getRawParameterValue(ezsqueeze::ParamID::body)->load();
    const float deSnapPct   = apvts.getRawParameterValue(ezsqueeze::ParamID::deSnap)->load();
    const bool  bypassed =
        apvts.getRawParameterValue(ezsqueeze::ParamID::bypass)->load() > 0.5f;

    if (bypassed)
        return;

    // Keep dry copy for wet/dry mixing
    juce::AudioBuffer<float> dryBuffer;
    const bool needsMix = mixPct < 99.9f;
    if (needsMix)
        dryBuffer.makeCopyOf(buffer);

    // ---- 2. Configure DSP modules with current parameter values ----

    cloudGainPreampL_.setGain(cloudGainDb);
    cloudGainPreampR_.setGain(cloudGainDb);
    auto impedanceType = static_cast<ezsqueeze::CloudGainPreamp::ImpedanceType>(impedanceIdx);
    cloudGainPreampL_.setImpedance(impedanceType);
    cloudGainPreampR_.setImpedance(impedanceType);

    auto detMode = static_cast<ezsqueeze::Detector::Mode>(detectorMode);
    detectorL_.setMode(detMode);
    detectorR_.setMode(detMode);

    sidechainFilterL_.setHPF(scHPFFreq);
    sidechainFilterR_.setHPF(scHPFFreq);
    sidechainFilterL_.setLPF(scLPFFreq);
    sidechainFilterR_.setLPF(scLPFFreq);

    stereoLink_.setLinkAmount(stereoLinkPct / 100.0f);
    stereoLink_.setMSMode(msMode);

    gainComputer_.setThreshold(thresholdDb);
    gainComputer_.setRatio(ratioVal);
    gainComputer_.setKnee(static_cast<ezsqueeze::GainComputer::KneeMode>(kneeIndex));

    transientSculptor_.setSnap(snapPct / 100.0f);
    transientSculptor_.setBody(bodyPct / 100.0f);
    transientSculptor_.setDeSnap(deSnapPct / 100.0f);

    float effectiveAttack  = transientSculptor_.getModifiedAttack(attackMs);
    float effectiveRelease = transientSculptor_.getModifiedRelease(releaseMs);

    envelopeFollowerL_.setAttack(effectiveAttack);
    envelopeFollowerR_.setAttack(effectiveAttack);
    envelopeFollowerL_.setRelease(effectiveRelease);
    envelopeFollowerR_.setRelease(effectiveRelease);

    if (autoReleaseOn)
    {
        programDependentRelease_.setFastRelease(effectiveRelease * 0.2f);
        programDependentRelease_.setSlowRelease(effectiveRelease);
    }

    vibeWheelL_.setAmount(vibeAmount / 100.0f);
    vibeWheelR_.setAmount(vibeAmount / 100.0f);

    dualStageCompressor_.setEnabled(dualStackOn);

    // ---- 3. Cloud-Gain preamp (per-sample, before oversampling) ----

    {
        float* dataL = buffer.getWritePointer(0);
        float* dataR = buffer.getWritePointer(1);
        for (int i = 0; i < numSamples; ++i)
        {
            dataL[i] = cloudGainPreampL_.process(dataL[i]);
            dataR[i] = cloudGainPreampR_.process(dataR[i]);
        }
    }

    // ---- 4. Oversampling up ----

    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::AudioBlock<float> osBlock = block;

    const bool shouldOversample = osIndex > 0 && !ecoMode;
    if (shouldOversample)
        osBlock = oversampling_.upsample(block);

    double effectiveRate = shouldOversample
        ? oversampling_.getOversampledRate()
        : currentSampleRate_;
    lookaheadBufferL_.setDelay(lookaheadMs, effectiveRate);
    lookaheadBufferR_.setDelay(lookaheadMs, effectiveRate);

    // ---- 5. Per-sample processing loop ----

    const int osNumSamples = static_cast<int>(osBlock.getNumSamples());
    float* osL = osBlock.getChannelPointer(0);
    float* osR = osBlock.getChannelPointer(1);

    for (int i = 0; i < osNumSamples; ++i)
    {
        float L = osL[i];
        float R = osR[i];

        float procL = L, procR = R;
        if (msMode)
            ezsqueeze::StereoLink::encodeMidSide(L, R, procL, procR);

        float scL = sidechainFilterL_.process(procL);
        float scR = sidechainFilterR_.process(procR);

        float levelL = detectorL_.process(scL);
        float levelR = detectorR_.process(scR);

        stereoLink_.processLink(levelL, levelR);

        constexpr float DB_FLOOR = 1e-6f;
        float dbL = 20.0f * std::log10(std::max(levelL, DB_FLOOR));
        float dbR = 20.0f * std::log10(std::max(levelR, DB_FLOOR));

        float grL = gainComputer_.compute(dbL);
        float grR = gainComputer_.compute(dbR);

        if (autoReleaseOn)
        {
            float avgLevel = (levelL + levelR) * 0.5f;
            float avgGR    = (grL + grR) * 0.5f;
            float adaptiveRelease = programDependentRelease_.computeRelease(
                avgLevel, prevLevel_, avgGR);
            envelopeFollowerL_.setRelease(adaptiveRelease);
            envelopeFollowerR_.setRelease(adaptiveRelease);
            prevLevel_ = avgLevel;
        }

        float smoothGRL = envelopeFollowerL_.process(grL);
        float smoothGRR = envelopeFollowerR_.process(grR);

        autoMakeup_.updateAdaptive((smoothGRL + smoothGRR) * 0.5f);

        float delayedL = lookaheadBufferL_.process(procL);
        float delayedR = lookaheadBufferR_.process(procR);

        float gainLinL = std::pow(10.0f, -smoothGRL / 20.0f);
        float gainLinR = std::pow(10.0f, -smoothGRR / 20.0f);
        procL = delayedL * gainLinL;
        procR = delayedR * gainLinR;

        dualStageCompressor_.process(procL, procR);

        transientSculptor_.processDeSnap(procL, procR);

        vibeWheelL_.process(procL);
        vibeWheelR_.process(procR);

        if (msMode)
        {
            float outL, outR;
            ezsqueeze::StereoLink::decodeMidSide(procL, procR, outL, outR);
            procL = outL;
            procR = outR;
        }

        osL[i] = procL;
        osR[i] = procR;
    }

    // ---- 6. Downsample ----

    if (shouldOversample)
        oversampling_.downsample(block);

    // ---- 7. Mix dry/wet ----

    if (needsMix)
    {
        const float wet = mixPct / 100.0f;
        const float dry = 1.0f - wet;

        for (int ch = 0; ch < numChannels; ++ch)
        {
            auto* wetData = buffer.getWritePointer(ch);
            auto* dryData = dryBuffer.getReadPointer(ch);

            for (int i = 0; i < numSamples; ++i)
                wetData[i] = wetData[i] * wet + dryData[i] * dry;
        }
    }

    // ---- 8. Makeup gain ----

    if (autoMakeupOn)
    {
        float staticGainDb   = autoMakeup_.computeStatic(thresholdDb, ratioVal);
        float adaptiveGainDb = autoMakeup_.getGainDB();
        float totalMakeupDb  = (staticGainDb + adaptiveGainDb) * 0.5f;
        buffer.applyGain(juce::Decibels::decibelsToGain(totalMakeupDb));
    }
    else if (makeupDb > 0.01f)
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

#pragma once

#include "Utilities.h"
#include "Detector.h"
#include "GainComputer.h"
#include "EnvelopeFollower.h"
#include "LookaheadBuffer.h"
#include "StereoLink.h"
#include "SidechainFilter.h"
#include "AutoMakeup.h"
#include "ProgramDependentRelease.h"
#include "ParallelMix.h"

namespace ezsqueeze::dsp {

struct CompressorParameters {
    float thresholdDecibels { -18.0f };
    float ratio { 4.0f };
    KneeType knee { KneeType::Soft };

    float attackMs { 5.0f };
    float releaseMs { 150.0f };

    float lookaheadMs { 0.0f };

    float scHpfHz { 80.0f };
    float scLpfHz { 8000.0f };
    bool scHpfEnabled { true };
    bool scLpfEnabled { false };

    float stereoLinkAmount { 1.0f }; // 0..1

    bool useProgramDependentRelease { false };
    bool useAutoMakeup { false };
    bool useMidSide { false };

    float parallelMix { 1.0f };
};

class CompressorCore {
public:
    void prepare(const double newSampleRate, const std::size_t numChannels) {
        sampleRate = newSampleRate;
        channels = std::max<std::size_t>(1, numChannels);

        detectorLeft.prepare(sampleRate);
        detectorRight.prepare(sampleRate);
        detectorLeft.setMode(DetectorMode::RMS);
        detectorRight.setMode(DetectorMode::RMS);

        lookahead.prepare(sampleRate, channels);

        sidechain.prepare(sampleRate);
        sidechain.setHPFFrequencyHz(parameters.scHpfHz);
        sidechain.setLPFFrequencyHz(parameters.scLpfHz);
        sidechain.setHPFEnabled(parameters.scHpfEnabled);
        sidechain.setLPFEnabled(parameters.scLpfEnabled);

        programRelease.prepare(sampleRate);

    }

    void reset() {
        detectorLeft.reset();
        detectorRight.reset();
        lookahead.reset();
        sidechain.reset();
        autoMakeup.reset();
    }

    void setParameters(const CompressorParameters &newParams) {
        parameters = newParams;
        lookahead.setDelayMilliseconds(parameters.lookaheadMs);
        sidechain.setHPFEnabled(parameters.scHpfEnabled);
        sidechain.setLPFEnabled(parameters.scLpfEnabled);
        sidechain.setHPFFrequencyHz(parameters.scHpfHz);
        sidechain.setLPFFrequencyHz(parameters.scLpfHz);
        stereoLink.setLinkAmount(parameters.stereoLinkAmount);
        detectorLeft.setEnvelopeTimesMilliseconds(parameters.attackMs, parameters.releaseMs);
        detectorRight.setEnvelopeTimesMilliseconds(parameters.attackMs, parameters.releaseMs);
    }

    int getLatencySamples() const noexcept { return lookahead.getLatencySamples(); }

    void processBlock(float **audioData, const std::size_t numChannelsInBlock, const int numSamples) noexcept {
        if (numChannelsInBlock == 0 || numSamples <= 0) return;

        for (int n = 0; n < numSamples; ++n) {
            const float inL = (numChannelsInBlock >= 1) ? audioData[0][n] : 0.0f;
            const float inR = (numChannelsInBlock >= 2) ? audioData[1][n] : inL;

            // Sidechain path
            const float scL = sidechain.processSample(inL);
            const float scR = sidechain.processSample(inR);

            float grDB_A = 0.0f, grDB_B = 0.0f; // domain A/B: L/R or M/S
            float dryA = 0.0f, dryB = 0.0f;     // delayed domain samples

            if (parameters.useMidSide) {
                // Detect on M/S
                float scM, scS; MidSide::encode(scL, scR, scM, scS);
                const float detM = detectorLeft.processSampleLinear(scM);
                const float detS = detectorRight.processSampleLinear(scS);

                float ctrlM = detM, ctrlS = detS;
                stereoLink.process(detM, detS, ctrlM, ctrlS);

                const float ctrlDB_M = linearToDecibels(ctrlM);
                const float ctrlDB_S = linearToDecibels(ctrlS);

                if (parameters.useProgramDependentRelease) {
                    const float avgCtrlLin = 0.5f * (ctrlM + ctrlS);
                    (void) programRelease.updateSuggestedReleaseMs(avgCtrlLin, lastGRDecibels);
                }

                grDB_A = GainComputer::computeGainReductionDecibels(ctrlDB_M, parameters.thresholdDecibels, parameters.ratio, parameters.knee);
                grDB_B = GainComputer::computeGainReductionDecibels(ctrlDB_S, parameters.thresholdDecibels, parameters.ratio, parameters.knee);

                // Delay audio, then encode to M/S for gain application
                const float dl = lookahead.processSample(0, inL);
                const float dr = lookahead.processSample(1, inR);
                MidSide::encode(dl, dr, dryA, dryB); // A=Mid, B=Side
            } else {
                // L/R domain
                const float detL = detectorLeft.processSampleLinear(scL);
                const float detR = detectorRight.processSampleLinear(scR);

                float ctrlL, ctrlR; stereoLink.process(detL, detR, ctrlL, ctrlR);
                const float ctrlDB_L = linearToDecibels(ctrlL);
                const float ctrlDB_R = linearToDecibels(ctrlR);

                if (parameters.useProgramDependentRelease) {
                    const float avgCtrlLin = 0.5f * (ctrlL + ctrlR);
                    (void) programRelease.updateSuggestedReleaseMs(avgCtrlLin, lastGRDecibels);
                }

                grDB_A = GainComputer::computeGainReductionDecibels(ctrlDB_L, parameters.thresholdDecibels, parameters.ratio, parameters.knee);
                grDB_B = GainComputer::computeGainReductionDecibels(ctrlDB_R, parameters.thresholdDecibels, parameters.ratio, parameters.knee);

                // Delay audio in L/R domain
                dryA = lookahead.processSample(0, inL);
                dryB = lookahead.processSample(1, inR);
            }

            lastGRDecibels = 0.5f * (grDB_A + grDB_B);
            const float gainA = GainComputer::reductionDecibelsToLinearGain(grDB_A);
            const float gainB = GainComputer::reductionDecibelsToLinearGain(grDB_B);

            float wetA = dryA * gainA;
            float wetB = dryB * gainB;

            // Decode back to L/R if we were in M/S domain
            float wetL = wetA, wetR = wetB;
            float dryL = dryA, dryR = dryB;
            if (parameters.useMidSide) {
                MidSide::decode(wetA, wetB, wetL, wetR);
                MidSide::decode(dryA, dryB, dryL, dryR);
            }

            // Parallel mix in L/R domain
            parallel.setMix(parameters.parallelMix);
            const float outL = parallel.processSample(dryL, wetL);
            const float outR = parallel.processSample(dryR, wetR);

            // Auto makeup (post)
            float makeupDB = 0.0f;
            if (parameters.useAutoMakeup) {
                autoMakeup.updateAdaptive(lastGRDecibels);
                makeupDB = autoMakeup.getAdaptiveMakeupDecibels();
            }
            const float makeupLinear = decibelsToLinear(makeupDB);

            if (numChannelsInBlock >= 1) audioData[0][n] = outL * makeupLinear;
            if (numChannelsInBlock >= 2) audioData[1][n] = outR * makeupLinear;
        }
    }

private:
    double sampleRate { 44100.0 };
    std::size_t channels { 2 };

    CompressorParameters parameters {};

    DetectorEngine detectorLeft {};
    DetectorEngine detectorRight {};
    LookaheadBuffer lookahead {};
    StereoLinker stereoLink {};
    SidechainFilter sidechain {};

    AutoMakeupGain autoMakeup {};
    ProgramDependentRelease programRelease {};
    ParallelMixer parallel {};

    float lastGRDecibels { 0.0f };
};

} // namespace ezsqueeze::dsp

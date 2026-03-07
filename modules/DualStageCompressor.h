#pragma once

#include "../source/dsp/Detector.h"
#include "../source/dsp/GainComputer.h"
#include "../source/dsp/EnvelopeFollower.h"

#include <cmath>
#include <algorithm>

namespace ezsqueeze
{

/**
 * @brief FET-fast + Opto-slow serial dual-stage compressor.
 *
 * Two independent compression stages run in series:
 *  - **FET stage**: fast attack (0.1–5 ms), fast release (50–150 ms),
 *    ratio 3:1–8:1. Catches transients.
 *  - **Opto stage**: slow attack (5–30 ms), slow release (200–1000 ms),
 *    ratio 1.5:1–4:1. Smooth body levelling.
 *
 * Each stage has its own Detector → GainComputer → EnvelopeFollower chain.
 * Stereo detection is linked (max of L/R).
 */
class DualStageCompressor
{
public:
    /** @brief Prepare all internal DSP for the given sample rate. */
    void setSampleRate(double sr)
    {
        sampleRate_ = sr;
        fetDetectorL_.setSampleRate(sr);
        fetDetectorR_.setSampleRate(sr);
        fetEnvelope_.setSampleRate(sr);
        optoDetectorL_.setSampleRate(sr);
        optoDetectorR_.setSampleRate(sr);
        optoEnvelope_.setSampleRate(sr);

        applyDefaults();
    }

    /** @brief Enable or disable the dual-stage compressor. */
    void setEnabled(bool on) { enabled_ = on; }

    // ── FET stage parameters ────────────────────────────────────────

    /** @brief Set FET threshold in dB. */
    void setFetThreshold(float dB) { fetComputer_.setThreshold(dB); }

    /** @brief Set FET ratio, clamped to 3–8. */
    void setFetRatio(float r) { fetComputer_.setRatio(std::clamp(r, 3.0f, 8.0f)); }

    /** @brief Set FET attack in ms, clamped to 0.1–5. */
    void setFetAttack(float ms) { fetEnvelope_.setAttack(std::clamp(ms, 0.1f, 5.0f)); }

    /** @brief Set FET release in ms, clamped to 50–150. */
    void setFetRelease(float ms) { fetEnvelope_.setRelease(std::clamp(ms, 50.0f, 150.0f)); }

    /** @brief Set FET knee mode. */
    void setFetKnee(GainComputer::KneeMode k) { fetComputer_.setKnee(k); }

    // ── Opto stage parameters ───────────────────────────────────────

    /** @brief Set Opto threshold in dB. */
    void setOptoThreshold(float dB) { optoComputer_.setThreshold(dB); }

    /** @brief Set Opto ratio, clamped to 1.5–4. */
    void setOptoRatio(float r) { optoComputer_.setRatio(std::clamp(r, 1.5f, 4.0f)); }

    /** @brief Set Opto attack in ms, clamped to 5–30. */
    void setOptoAttack(float ms) { optoEnvelope_.setAttack(std::clamp(ms, 5.0f, 30.0f)); }

    /** @brief Set Opto release in ms, clamped to 200–1000. */
    void setOptoRelease(float ms) { optoEnvelope_.setRelease(std::clamp(ms, 200.0f, 1000.0f)); }

    /** @brief Set Opto knee mode. */
    void setOptoKnee(GainComputer::KneeMode k) { optoComputer_.setKnee(k); }

    // ── Processing ──────────────────────────────────────────────────

    /**
     * @brief Process one stereo sample pair through both stages.
     * @param[in,out] left   Left channel sample.
     * @param[in,out] right  Right channel sample.
     */
    void process(float& left, float& right)
    {
        if (!enabled_)
            return;

        applyStage(left, right,
                    fetDetectorL_, fetDetectorR_,
                    fetComputer_, fetEnvelope_);

        applyStage(left, right,
                    optoDetectorL_, optoDetectorR_,
                    optoComputer_, optoEnvelope_);
    }

    /** @brief Get the last FET gain reduction in dB (positive). */
    float getFetGR() const { return lastFetGR_; }

    /** @brief Get the last Opto gain reduction in dB (positive). */
    float getOptoGR() const { return lastOptoGR_; }

    /** @brief Reset all internal state. */
    void reset()
    {
        fetDetectorL_.reset();  fetDetectorR_.reset();
        optoDetectorL_.reset(); optoDetectorR_.reset();
        fetEnvelope_.reset();
        optoEnvelope_.reset();
        lastFetGR_ = 0.0f;
        lastOptoGR_ = 0.0f;
    }

private:
    static constexpr float DB_FLOOR = 1e-6f;

    void applyDefaults()
    {
        fetDetectorL_.setMode(Detector::Mode::Peak);
        fetDetectorR_.setMode(Detector::Mode::Peak);
        fetDetectorL_.setWindowSize(5.0f);
        fetDetectorR_.setWindowSize(5.0f);

        optoDetectorL_.setMode(Detector::Mode::RMS);
        optoDetectorR_.setMode(Detector::Mode::RMS);
        optoDetectorL_.setWindowSize(30.0f);
        optoDetectorR_.setWindowSize(30.0f);

        fetComputer_.setThreshold(-20.0f);
        fetComputer_.setRatio(4.0f);
        fetComputer_.setKnee(GainComputer::KneeMode::Hard);

        optoComputer_.setThreshold(-15.0f);
        optoComputer_.setRatio(2.0f);
        optoComputer_.setKnee(GainComputer::KneeMode::Soft);

        fetEnvelope_.setAttack(0.5f);
        fetEnvelope_.setRelease(100.0f);

        optoEnvelope_.setAttack(15.0f);
        optoEnvelope_.setRelease(500.0f);
    }

    void applyStage(float& left, float& right,
                    Detector& detL, Detector& detR,
                    GainComputer& computer, EnvelopeFollower& envelope)
    {
        float levelL = detL.process(left);
        float levelR = detR.process(right);

        float peakLinear = std::max(levelL, levelR);
        float peakDB = 20.0f * std::log10(std::max(peakLinear, DB_FLOOR));

        float grRaw = computer.compute(peakDB);
        float grSmooth = envelope.process(grRaw);

        if (&computer == &fetComputer_)
            lastFetGR_ = grSmooth;
        else
            lastOptoGR_ = grSmooth;

        float gainLinear = std::pow(10.0f, -grSmooth / 20.0f);
        left  *= gainLinear;
        right *= gainLinear;
    }

    double sampleRate_ = 44100.0;
    bool enabled_ = true;

    Detector fetDetectorL_, fetDetectorR_;
    GainComputer fetComputer_;
    EnvelopeFollower fetEnvelope_;
    float lastFetGR_ = 0.0f;

    Detector optoDetectorL_, optoDetectorR_;
    GainComputer optoComputer_;
    EnvelopeFollower optoEnvelope_;
    float lastOptoGR_ = 0.0f;
};

} // namespace ezsqueeze

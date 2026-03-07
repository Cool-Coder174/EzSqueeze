# EzSqueeze Technical Notes

**Version:** 1.0.0  
**Last Updated:** March 2026  
**Author:** Isaac Hernandez

---

## Overview

This document provides in-depth technical details about EzSqueeze's DSP algorithms, implementation strategies, and design decisions. It serves as a reference for developers working on the codebase and users interested in the underlying technology.

---

## Table of Contents

1. [Architecture Overview](#architecture-overview)
2. [Signal Flow](#signal-flow)
3. [Detector Engine](#detector-engine)
4. [Gain Computer](#gain-computer)
5. [Envelope Follower](#envelope-follower)
6. [Lookahead Buffer](#lookahead-buffer)
7. [Stereo Linking & M/S Processing](#stereo-linking--ms-processing)
8. [Sidechain Filtering](#sidechain-filtering)
9. [Oversampling](#oversampling)
10. [Auto-Makeup Gain](#auto-makeup-gain)
11. [Program-Dependent Release](#program-dependent-release)
12. [Dual-Stage Compression](#dual-stage-compression)
13. [Cloud-Gain Preamp](#cloud-gain-preamp)
14. [Transient Sculptor](#transient-sculptor)
15. [Saturation & Harmonic Coloration](#saturation--harmonic-coloration)
16. [Latency Compensation](#latency-compensation)
17. [Performance Considerations](#performance-considerations)
18. [References](#references)

---

## Architecture Overview

EzSqueeze is implemented as a modular DSP chain where each processing stage is an independent, header-only C++ class in the `ezsqueeze` namespace under `source/dsp/`. This design enables unit testing of each component in isolation, compile-time inlining for performance, and straightforward reuse.

The high-level signal chain:

```
Input → Cloud-Gain Preamp → Oversampling Upsample ↓
                                                     
Sidechain Tap → SC Filter → Detector → Gain Computer → Envelope Follower
                                                                  ↓
                          Lookahead Buffer ← Audio Path       Gain Reduction
                                ↓                                  ↓
                          GR Application ← ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ┘
                                ↓
                          Mix (dry/wet) → Makeup Gain → Oversampling Downsample → Output
```

### Module Inventory

| Class | Header | Responsibility |
|-------|--------|---------------|
| `Detector` | `Detector.h` | Peak/RMS level measurement, dB conversion |
| `GainComputer` | `GainComputer.h` | Compression curve, threshold/ratio/knee math |
| `EnvelopeFollower` | `EnvelopeFollower.h` | Attack/release smoothing of gain reduction |
| `LookaheadBuffer` | `LookaheadBuffer.h` | Delay line for look-ahead peak catching |
| `StereoLink` | `StereoLink.h` | Stereo linking and M/S encode/decode |
| `SidechainFilter` | `SidechainFilter.h` | HPF/LPF on detection signal |
| `AutoMakeup` | `AutoMakeup.h` | Automatic makeup gain estimation |
| `ProgramDependentRelease` | `ProgramDependentRelease.h` | Adaptive release time control |

All classes follow a common interface pattern:

```cpp
class Module {
public:
    void prepare(double sampleRate, int samplesPerBlock = 512);
    void reset();
    float process(float input);
};
```

---

## Signal Flow

### Main Processing Path

The per-sample processing order is:

1. **Input Stage:** Apply Cloud-Gain preamp and impedance character.
2. **Oversampling Upsample:** If enabled, upsample the audio by 2x/4x/8x using polyphase IIR filters.
3. **Sidechain Tap:** Copy the audio (or external sidechain) to the detection path.
4. **Sidechain Filter:** Apply HPF/LPF to the detection signal to shape what frequencies drive compression.
5. **Detector:** Convert the filtered sidechain to a level envelope (Peak or RMS), then to decibels.
6. **Gain Computer:** Calculate the required gain reduction in dB based on threshold, ratio, and knee.
7. **Envelope Follower:** Smooth the gain reduction with attack/release time constants.
8. **Lookahead Buffer:** Delay the audio path so the detector can react before the transient arrives.
9. **Gain Application:** Convert smoothed GR from dB to linear and multiply with the delayed audio.
10. **Mix:** Blend compressed and dry signals: `output = dry * (1 - mix) + wet * mix`.
11. **Makeup Gain:** Apply manual or auto makeup gain.
12. **Oversampling Downsample:** If enabled, downsample back to the original rate with anti-aliasing.
13. **Output:** Final output to DAW.

### Per-Sample Processing (Pseudo-Code)

```cpp
void processBlock(float* left, float* right, int numSamples) {
    for (int i = 0; i < numSamples; ++i) {
        float scL = sidechainFilter.process(left[i]);
        float scR = sidechainFilter.process(right[i]);

        float levelL = detector.process(scL);
        float levelR = detector.process(scR);
        float dbL = detector.toDecibels(levelL);
        float dbR = detector.toDecibels(levelR);

        auto [linkL, linkR] = stereoLink.processLink(dbL, dbR);

        float grL = gainComputer.computeGainReduction(linkL);
        float grR = gainComputer.computeGainReduction(linkR);

        float smoothL = envelope.process(std::abs(grL));
        float smoothR = envelope.process(std::abs(grR));

        float delayedL = lookahead.process(left[i]);
        float delayedR = lookahead.process(right[i]);

        float gainLinL = std::pow(10.0f, -smoothL / 20.0f);
        float gainLinR = std::pow(10.0f, -smoothR / 20.0f);

        float wetL = delayedL * gainLinL;
        float wetR = delayedR * gainLinR;

        left[i]  = delayedL * (1.0f - mix) + wetL * mix;
        right[i] = delayedR * (1.0f - mix) + wetR * mix;

        left[i]  *= makeupGainLinear;
        right[i] *= makeupGainLinear;
    }
}
```

### Dual-Stage Mode

When enabled, the signal passes through two serial compressor instances. The first stage uses FET-style fast timing, and the second uses opto-style slow timing:

```
Input → Stage 1 (FET: fast attack, short release, higher ratio)
      → Stage 2 (Opto: slow attack, long release, lower ratio)
      → Output
```

Both stages share the same `Detector`, `GainComputer`, `EnvelopeFollower` class implementations but with different parameter values. The threshold is split between stages to avoid over-compression:

```cpp
float stage1Threshold = threshold + 3.0f;  // tighter, catches peaks
float stage2Threshold = threshold - 3.0f;  // looser, smooths average
```

---

## Detector Engine

### Peak Detector

Tracks the instantaneous absolute peak of the input signal. Useful for transient-heavy material where the compressor needs to respond to individual sample peaks.

```cpp
float Detector::processPeak(float input) {
    float absInput = std::abs(input);
    if (absInput > peakHold) {
        peakHold = absInput;
    } else {
        peakHold *= decayCoeff;  // Exponential decay
        if (absInput > peakHold)
            peakHold = absInput;
    }
    return peakHold;
}
```

The decay coefficient provides a brief hold-and-release behavior to prevent the detector from dropping to zero between samples in a waveform cycle:

```
decayCoeff = 1.0 - e^(-1 / (holdTimeMs * 0.001 * sampleRate))
```

Typical hold time: 1-5 ms.

### RMS Detector

Calculates the root-mean-square (average power) level using a one-pole lowpass filter on the squared input. This gives a smoother, more "musical" level reading.

```cpp
float Detector::processRMS(float input) {
    float inputSquared = input * input;
    rmsSquared = (1.0f - rmsAlpha) * rmsSquared + rmsAlpha * inputSquared;
    return std::sqrt(rmsSquared);
}
```

The RMS averaging coefficient `rmsAlpha` is derived from the desired window length:

```
windowSamples = windowMs * 0.001 * sampleRate
rmsAlpha = 1.0 - e^(-1.0 / windowSamples)
```

Default window: 5 ms. Shorter windows (1-2 ms) approach peak behavior; longer windows (10-50 ms) give a very averaged reading.

### Conversion to Decibels

Linear amplitude is converted to decibels using a floor to prevent `log10(0)`:

```cpp
float Detector::toDecibels(float level) const {
    constexpr float kFloor = 1e-6f;  // -120 dBFS
    return 20.0f * std::log10(std::max(level, kFloor));
}
```

The floor of `1e-6` corresponds to -120 dBFS, which is well below any audible signal and prevents numerical underflow in the gain computer.

### Key Implementation Details

- **Thread safety:** The detector maintains per-channel state (`peakHold`, `rmsSquared`). For stereo, two instances are used.
- **Reset behavior:** `reset()` zeros all state to avoid startup transients when the plugin is first enabled.
- **Mode switching:** Changing between Peak and RMS mid-stream is supported; the new mode starts from the current state to avoid clicks.

---

## Gain Computer

### Basic Compression Curve

The gain computer maps input level (in dB) to gain reduction (in dB). Below the threshold, output equals input (no reduction). Above the threshold, the output is compressed by the ratio.

For input level \(L_{in}\) (dB), threshold \(T\) (dB), and ratio \(R\):

```
overshoot = L_in - T

if overshoot <= 0:
    GR = 0
else:
    output = T + overshoot / R
    GR = output - L_in = overshoot * (1/R - 1)
```

Implementation:

```cpp
float GainComputer::computeGainReduction(float inputDB) {
    if (inputDB <= threshold) return 0.0f;

    float overshoot = inputDB - threshold;
    return overshoot * (1.0f / ratio - 1.0f);  // Negative value
}
```

**Example:** Input = -10 dB, Threshold = -20 dB, Ratio = 4:1  
Overshoot = 10 dB → GR = 10 × (0.25 - 1) = -7.5 dB

### Knee Implementation

#### Hard Knee (0 dB width)

An abrupt transition at the threshold. Below → no compression. Above → full ratio. This produces a sharp "break" in the transfer curve.

```cpp
// Hard knee is simply the basic formula above
if (inputDB <= threshold) return 0.0f;
float overshoot = inputDB - threshold;
return overshoot * (1.0f / ratio - 1.0f);
```

#### Soft Knee (6 dB width)

A parabolic interpolation zone centered on the threshold. Within the knee region, compression gradually increases from 1:1 to the full ratio.

For knee width \(W\):
- Below knee: \(L_{in} < T - W/2\) → no compression
- Above knee: \(L_{in} > T + W/2\) → full ratio
- Inside knee: parabolic blend

```cpp
float GainComputer::computeWithKnee(float inputDB) {
    float halfKnee = kneeWidth * 0.5f;
    
    if (inputDB < threshold - halfKnee) {
        return 0.0f;
    }
    
    if (inputDB > threshold + halfKnee) {
        float overshoot = inputDB - threshold;
        return overshoot * (1.0f / ratio - 1.0f);
    }
    
    // Parabolic region
    float x = inputDB - threshold + halfKnee;
    float normalizedX = x / kneeWidth;
    float overshoot = inputDB - threshold;
    float fullGR = overshoot * (1.0f / ratio - 1.0f);
    
    // Quadratic interpolation: GR ramps from 0 at knee start to full at knee end
    return normalizedX * normalizedX * fullGR / (normalizedX + (1.0f - normalizedX));
}
```

An equivalent formulation from the Giannoulis et al. paper uses the full parabolic knee equation:

```
x = inputDB - threshold + W/2

GR = (1/R - 1) * x² / (2W)    for 0 ≤ x ≤ W
```

This guarantees:
- At \(T - W/2\): GR = 0 (seamless entry)
- At \(T\): GR = half of full-ratio GR (midpoint)
- At \(T + W/2\): GR matches full-ratio formula (seamless exit)

#### Medium Knee (3 dB width)

Same algorithm as soft knee with `kneeWidth = 3.0f`.

### Ratio Extremes

- **Ratio = 1:1:** \(1/R - 1 = 0\), so GR is always zero regardless of input level. The compressor is effectively bypassed.
- **Ratio = ∞:** \(1/R - 1 = -1\), so GR equals the full overshoot. Output is clamped to the threshold, producing a brick-wall limiter.

### Mathematical Derivation

The compression transfer function:

```
y = T + (x - T) / R     for x > T
y = x                    for x ≤ T
```

Where x = input level (dB), y = output level (dB), T = threshold, R = ratio.

Gain reduction:
```
GR = y - x = T + (x - T)/R - x = (x - T)(1/R - 1) = Δ(1 - R)/R
```

Where Δ = x - T is the overshoot.

---

## Envelope Follower

### Attack/Release Implementation

A one-pole IIR lowpass filter with separate coefficients for rising (attack) and falling (release) signals. This is the industry-standard approach used in virtually all digital compressors.

```cpp
float EnvelopeFollower::process(float input) {
    float coeff;
    if (input > envelope) {
        coeff = attackCoeff;  // Signal rising → attack
    } else {
        coeff = releaseCoeff; // Signal falling → release
    }
    envelope += coeff * (input - envelope);
    return envelope;
}
```

This is equivalent to:
```
y[n] = y[n-1] + α * (x[n] - y[n-1])
     = (1 - α) * y[n-1] + α * x[n]
```

Where α is `attackCoeff` or `releaseCoeff` depending on direction.

### Time Constant Calculation

The coefficient α is derived from the desired time constant τ (in seconds) at the given sample rate:

```cpp
float EnvelopeFollower::timeToCoeff(float timeMs, double sampleRate) {
    if (timeMs <= 0.0f) return 1.0f;  // Instant response
    float timeSec = timeMs * 0.001f;
    float tau = timeSec * sampleRate;
    return 1.0f - std::exp(-1.0f / tau);
}
```

**Derivation:** For a first-order IIR system, the step response reaches \(1 - e^{-1} \approx 63.2\%\) of the target after τ samples. The time constant in samples is:

```
τ = t_ms * 0.001 * sampleRate
α = 1 - e^(-1/τ)
```

This means:
- After 1 × attack time: output reaches ~63% of the target
- After 3 × attack time: output reaches ~95%
- After 5 × attack time: output reaches ~99.3%

### Behavior Characteristics

**Fast attack + slow release (typical compressor):**
- The envelope quickly rises to meet transients
- Then slowly decays back, providing smooth gain restoration
- This is the classic compressor behavior: grab peaks fast, release gradually

**Equal attack and release:**
- Symmetrical response, more like a simple lowpass on the GR signal
- Less common but useful for some creative effects

### Sample Rate Independence

The time-to-coefficient conversion inherently handles sample rate changes. A 10 ms attack time produces:
- At 44100 Hz: α = 0.00226
- At 96000 Hz: α = 0.00104

Both reach 63% at exactly 10 ms of real time.

---

## Lookahead Buffer

### Purpose

The lookahead buffer delays the audio signal relative to the sidechain detection signal. This gives the detector and envelope time to react before the transient arrives in the audio path, enabling:

- Zero-overshoot compression on fast transients
- More transparent compression with slower attack settings
- Reduced distortion from extremely fast attack times

### Implementation

A simple circular buffer (delay line):

```cpp
class LookaheadBuffer {
    std::vector<float> buffer;
    int writePos = 0;
    int delaySamples = 0;

public:
    void setDelay(float delayMs, double sampleRate) {
        delaySamples = static_cast<int>(delayMs * 0.001 * sampleRate);
        if (delaySamples == 0) {
            buffer.clear();
            return;
        }
        buffer.assign(delaySamples + 1, 0.0f);
        writePos = 0;
    }

    float process(float input) {
        if (delaySamples == 0) return input;

        int bufSize = static_cast<int>(buffer.size());
        int readPos = (writePos - delaySamples + bufSize) % bufSize;
        float output = buffer[readPos];
        buffer[writePos] = input;
        writePos = (writePos + 1) % bufSize;
        return output;
    }

    int getLatencySamples() const { return delaySamples; }
};
```

### Delay Calculation

```
delaySamples = floor(delayMs × 0.001 × sampleRate)
```

Examples:
| Delay (ms) | Sample Rate | Delay (samples) |
|-----------|-------------|-----------------|
| 5 | 44100 | 220 |
| 5 | 48000 | 240 |
| 10 | 96000 | 960 |

### Zero-Delay Mode

When lookahead is 0 ms, the buffer is bypassed entirely — `process()` returns the input directly. This avoids unnecessary memory access and the buffer is not allocated.

### Latency Reporting

The lookahead delay must be reported to the DAW host for plugin delay compensation (PDC):

```cpp
int getLatencySamples() const {
    return lookaheadBuffer.getLatencySamples() + oversamplingLatency;
}
```

The DAW delays all other tracks by this amount so everything stays aligned.

---

## Stereo Linking & M/S Processing

### Stereo Link

Stereo linking blends each channel's detected level with the maximum of both channels. This ensures both channels receive the same gain reduction, preserving the stereo image.

```cpp
std::pair<float, float> StereoLink::processLink(float leftLevel, float rightLevel) {
    float maxLevel = std::max(leftLevel, rightLevel);
    float leftOut  = leftLevel  + linkAmount * (maxLevel - leftLevel);
    float rightOut = rightLevel + linkAmount * (maxLevel - rightLevel);
    return {leftOut, rightOut};
}
```

This is a linear interpolation (`lerp`):
```
output_L = L + link × (max(L, R) - L)
output_R = R + link × (max(L, R) - R)
```

At `linkAmount = 0`: each channel is independent.  
At `linkAmount = 1`: both channels use `max(L, R)` → identical GR.

**Why max() instead of average?** Using the maximum ensures that a loud event in either channel triggers compression in both. An average would under-compress when one channel is significantly louder, potentially allowing one side to clip while the other is compressed.

### M/S Encoding

Convert left/right to mid/side for independent processing:

```cpp
std::pair<float, float> StereoLink::encodeMS(float left, float right) {
    float mid  = (left + right) * 0.5f;
    float side = (left - right) * 0.5f;
    return {mid, side};
}
```

**Mid** represents the center image (mono content).  
**Side** represents the stereo difference (width content).

### M/S Decoding

Reconstruct left/right from processed mid/side:

```cpp
std::pair<float, float> StereoLink::decodeMS(float mid, float side) {
    float left  = mid + side;
    float right = mid - side;
    return {left, right};
}
```

### Roundtrip Proof

Encode then decode must be lossless:
```
M = (L + R) / 2
S = (L - R) / 2

L' = M + S = (L + R)/2 + (L - R)/2 = L
R' = M - S = (L + R)/2 - (L - R)/2 = R
```

The 0.5 scaling in the encode step is compensated by the addition/subtraction in the decode step, so the roundtrip is perfectly transparent with no gain change.

---

## Sidechain Filtering

### Purpose

The sidechain filters shape the detection signal without affecting the audio path. This controls which frequencies drive the compressor's gain reduction.

### Biquad Implementation

Both HPF and LPF use second-order biquad filters (Transposed Direct Form II) based on the Robert Bristow-Johnson Audio EQ Cookbook.

The general biquad difference equation:

```
y[n] = b0*x[n] + b1*x[n-1] + b2*x[n-2] - a1*y[n-1] - a2*y[n-2]
```

State-variable form (Transposed Direct Form II) for numerical stability:

```cpp
float SidechainFilter::processBiquad(float input, BiquadState& s) {
    float output = s.b0 * input + s.z1;
    s.z1 = s.b1 * input - s.a1 * output + s.z2;
    s.z2 = s.b2 * input - s.a2 * output;
    return output;
}
```

### High-Pass Filter Coefficients

Second-order Butterworth HPF at frequency \(f_c\):

```
ω0 = 2π × f_c / sampleRate
α  = sin(ω0) / (2 × Q)          // Q = 0.7071 for Butterworth

b0 = (1 + cos(ω0)) / 2
b1 = -(1 + cos(ω0))
b2 = (1 + cos(ω0)) / 2
a0 = 1 + α
a1 = -2 × cos(ω0)
a2 = 1 - α
```

All coefficients are normalized by dividing by a0.

### Low-Pass Filter Coefficients

Second-order Butterworth LPF at frequency \(f_c\):

```
ω0 = 2π × f_c / sampleRate
α  = sin(ω0) / (2 × Q)

b0 = (1 - cos(ω0)) / 2
b1 = 1 - cos(ω0)
b2 = (1 - cos(ω0)) / 2
a0 = 1 + α
a1 = -2 × cos(ω0)
a2 = 1 - α
```

### Cutoff Behavior

At the cutoff frequency, a second-order Butterworth filter attenuates by exactly -3.01 dB. The slope is -12 dB/octave (40 dB/decade) in the stopband.

### Bypass Condition

When HPF = 20 Hz and LPF ≥ 16000 Hz (essentially the full audible range), the filters have negligible effect. The implementation detects this and can skip filtering to save CPU.

### Frequency Stability

Filter coefficients are recalculated whenever the cutoff frequency or sample rate changes. Coefficient smoothing is applied over 32 samples to prevent zipper noise during parameter automation.

---

## Oversampling

### Purpose

Non-linear processing (saturation from Vibe Wheel, fast compression) generates harmonics that can fold back below the Nyquist frequency, causing aliasing. Oversampling mitigates this by:

1. **Upsampling** the audio to a higher rate (2×, 4×, 8×)
2. **Processing** the non-linear operations at the higher rate
3. **Downsampling** with an anti-aliasing filter to remove frequencies above the original Nyquist

### Implementation

EzSqueeze uses JUCE's built-in `juce::dsp::Oversampling<float>` class with half-band polyphase IIR filters:

```cpp
juce::dsp::Oversampling<float> oversampler(
    2,                                                          // numChannels
    oversamplingOrder,                                          // 1=2x, 2=4x, 3=8x
    juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, // filter type
    true                                                        // max quality
);
```

### Processing Flow

```cpp
void processBlock(juce::AudioBuffer<float>& buffer) {
    auto osBlock = oversampler.processSamplesUp(inputBlock);
    
    // All non-linear processing happens at oversampled rate
    processCompression(osBlock);
    processSaturation(osBlock);
    
    oversampler.processSamplesDown(inputBlock);
}
```

### Quality vs. CPU Trade-offs

| Factor | Stopband Rejection | CPU Overhead | Added Latency (samples @ original rate) |
|--------|-------------------|-------------|----------------------------------------|
| 2× | ~100 dB | +40-60% | ~8 |
| 4× | ~100 dB | +120-180% | ~16 |
| 8× | ~100 dB | +300-400% | ~32 |

### When to Oversample

- **Always recommended when:** Vibe Wheel > 30%, Cloud Gain > 10 dB, Impedance = Tube/Transformer
- **Usually not needed when:** Clean operation, Vibe = 0, Silicon impedance
- **Eco Mode:** Forces oversampling to 1× regardless of setting

---

## Auto-Makeup Gain

### Goal

Compensate for the volume loss caused by compression so the user can A/B compare without level differences biasing their judgment.

### Static Estimation

The basic approach estimates expected gain reduction from the threshold and ratio:

```cpp
float AutoMakeup::estimateStatic(float threshold, float ratio) {
    constexpr float kAssumedOvershoot = 10.0f;  // dB above threshold (typical)
    constexpr float kCompensation = 0.7f;       // 70% compensation factor
    
    float expectedGR = kAssumedOvershoot * (1.0f - 1.0f / ratio);
    return expectedGR * kCompensation;
}
```

The 10 dB assumed overshoot represents typical audio material. The 70% compensation factor avoids over-compensating — it's better to be slightly quiet than to clip.

**Example:** Threshold = -20 dB, Ratio = 4:1  
Expected GR = 10 × (1 - 0.25) = 7.5 dB  
Makeup = 7.5 × 0.7 = 5.25 dB

### Adaptive Estimation

For more accurate compensation, the adaptive mode measures actual gain reduction with a slow follower:

```cpp
float AutoMakeup::processAdaptive(float currentGR) {
    constexpr float kSmoothCoeff = 0.0001f;  // Very slow averaging
    constexpr float kAdaptiveFactor = 0.8f;
    
    averageGR += kSmoothCoeff * (std::abs(currentGR) - averageGR);
    return averageGR * kAdaptiveFactor;
}
```

The slow coefficient (~2 second time constant) prevents the makeup gain from "chasing" the music, which would undo the compression.

### Application

The makeup gain is applied after the mix stage:

```cpp
float totalMakeup = manualMakeup + (autoMakeupEnabled ? autoMakeupGain : 0.0f);
float makeupLinear = std::pow(10.0f, totalMakeup / 20.0f);
output *= makeupLinear;
```

---

## Program-Dependent Release

### Concept

Fixed release times are always a compromise: too fast causes distortion and "breathing" on sustained material, too slow causes pumping on transient material. Program-dependent release automatically adapts:

- **Transient material** (drums, plucked strings) → faster release to recover before the next hit
- **Sustained material** (pads, sustained vocals) → slower release for smooth, transparent compression

### Implementation

The adaptive algorithm uses a transient detector to distinguish between transient and sustained content:

```cpp
float ProgramDependentRelease::process(float inputLevel, float currentGR) {
    float transientAmount = std::abs(inputLevel - prevLevel);
    prevLevel = inputLevel;
    
    // Smooth the transient detection
    transientEnv += 0.01f * (transientAmount - transientEnv);
    
    float targetRelease;
    if (transientEnv > transientThreshold && std::abs(currentGR) > 3.0f) {
        targetRelease = fastRelease;   // 50 ms for transient content
    } else {
        targetRelease = slowRelease;   // 300 ms for sustained content
    }
    
    // Smooth the release time transition to avoid artifacts
    currentRelease += releaseSmoothing * (targetRelease - currentRelease);
    return currentRelease;
}
```

### Parameters

| Parameter | Value | Purpose |
|-----------|-------|---------|
| `fastRelease` | 50 ms | Recovery time for transient material |
| `slowRelease` | 300 ms | Recovery time for sustained material |
| `transientThreshold` | 6 dB | Sample-to-sample level change that triggers "transient" mode |
| `releaseSmoothing` | 0.001 | Prevents abrupt release time changes |

### Interaction with Manual Release

When auto-release is enabled, it modulates the manual release value:

```cpp
float effectiveRelease = manualRelease * adaptiveFactor;
// adaptiveFactor ranges from 0.3 (fast) to 1.5 (slow)
```

This respects the user's chosen release as a baseline while adapting around it.

---

## Dual-Stage Compression

### Architecture

Two complete compressor instances in series:

```
Input → [Stage 1: FET character] → [Stage 2: Opto character] → Output
```

### FET Stage (Stage 1 — Fast)

Emulates the fast response of a FET-based hardware compressor (e.g., 1176 style):

| Parameter | Range | Typical |
|-----------|-------|---------|
| Attack | 0.1 – 5 ms | 1 ms |
| Release | 50 – 150 ms | 80 ms |
| Ratio | 3:1 – 8:1 | 4:1 |
| Knee | Hard | — |

The FET stage catches fast transients and peak events, providing precise transient control.

### Opto Stage (Stage 2 — Slow)

Emulates the smooth, non-linear response of an optical compressor (e.g., LA-2A style):

| Parameter | Range | Typical |
|-----------|-------|---------|
| Attack | 5 – 30 ms | 15 ms |
| Release | 200 – 1000 ms | 400 ms |
| Ratio | 1.5:1 – 4:1 | 2:1 |
| Knee | Soft | — |

The opto stage provides smooth, musical leveling of the overall dynamic range.

### Threshold Splitting

To prevent over-compression, the user's threshold is distributed between stages:

```cpp
float stage1Thresh = userThreshold + 3.0f;  // Higher → catches only peaks
float stage2Thresh = userThreshold - 3.0f;  // Lower → acts on average level

float stage1Ratio = userRatio * 0.7f;       // Slightly lower per stage
float stage2Ratio = userRatio * 0.5f;       // Even gentler for smoothing
```

### Combined GR

Total gain reduction is the sum of both stages:
```
GR_total = GR_stage1 + GR_stage2
```

This is reported on the meter and used for auto-makeup calculation.

---

## Cloud-Gain Preamp

### Clean Gain Stage

Provides +0 to +30 dB of gain before the compressor input:

```cpp
float CloudGain::process(float input) {
    return input * gainLinear;
    // gainLinear = pow(10.0f, gainDB / 20.0f)
}
```

The gain is applied in the linear domain for maximum precision. The noise floor target is < -110 dBFS, achieved by using 64-bit intermediates where necessary.

### Impedance Character

Three impedance models add subtle harmonic coloration to the preamp output:

#### Silicon (Clean)
Minimal processing — effectively a wire with gain:
```cpp
float silicon(float x) {
    return x;  // No coloration
}
```

#### Tube (Even Harmonics)
Asymmetric soft clipping that generates predominantly even harmonics (2nd, 4th):

```cpp
float tube(float x, float drive) {
    float shaped = x + 0.05f * x * x;      // 2nd harmonic
    shaped += 0.01f * x * x * x * x;       // 4th harmonic
    return std::tanh(shaped * drive) / std::tanh(drive);  // Soft limit
}
```

The asymmetry of `x²` generates even-order harmonics, which are perceived as "warm" and "musical."

#### Transformer (Odd + Even)
Symmetric saturation with a subtle low-frequency bump:

```cpp
float transformer(float x, float drive) {
    float shaped = x + 0.03f * x * x;          // Even harmonics
    shaped += 0.08f * x * x * x;               // Odd harmonics (3rd)
    float limited = std::tanh(shaped * drive) / std::tanh(drive);
    
    // Subtle LF boost (transformer core characteristic)
    float lfBoost = lowShelf.process(limited);  // +0.5 dB @ 100 Hz
    return lfBoost;
}
```

The odd-harmonic content (`x³`) adds "grit," while the low-frequency shelf emulates the inductive coupling of a physical transformer core.

---

## Transient Sculptor

### Snap Control

Biases the effective attack time shorter to emphasize transients:

```cpp
float snapBias = 1.0f - (snap - 50.0f) / 100.0f;  // 0.5 (fast) to 1.5 (slow)
float effectiveAttack = baseAttack * snapBias;
```

At `snap = 100`: attack is halved → more transient emphasis.  
At `snap = 0`: attack is 1.5× longer → transients pass through more.  
At `snap = 50` (default): no modification.

### Body Control

Biases the effective release time longer to emphasize sustain:

```cpp
float bodyBias = 1.0f + (body - 50.0f) / 100.0f;  // 0.5 (short) to 1.5 (long)
float effectiveRelease = baseRelease * bodyBias;
```

At `body = 100`: release is 1.5× longer → sustain emphasis, pumping.  
At `body = 0`: release is halved → fast recovery, less sustain.  
At `body = 50` (default): no modification.

### De-Snap

A fast limiter applied to the detector output to soften transient peaks before they drive the gain computer:

```cpp
float TransientSculptor::deSnap(float detectorLevel) {
    if (deSnapAmount <= 0.0f) return detectorLevel;
    
    constexpr float kFastAttack = 0.1f;  // ms
    float limitThreshold = 1.0f - deSnapAmount * 0.5f;
    
    if (detectorLevel > limitThreshold) {
        float excess = detectorLevel - limitThreshold;
        detectorLevel = limitThreshold + excess * 0.1f;  // 10:1 limiting
    }
    return detectorLevel;
}
```

This is useful for softening pick noise on acoustic guitar, harsh consonants on vocals, or excessively sharp drum transients.

---

## Saturation & Harmonic Coloration

### Waveshaping Functions

The Vibe Wheel controls the amount of waveshaping applied to the audio signal. All waveshaping functions are designed to be smooth and differentiable to minimize aliasing.

#### Soft Clip (Tanh)

The primary saturation function using hyperbolic tangent:

```cpp
float softClip(float x, float drive) {
    return std::tanh(x * drive) / std::tanh(drive);
}
```

Properties:
- Smooth, odd-harmonic dominant saturation
- Output bounded to [-1, 1]
- The `/ tanh(drive)` normalization ensures unity gain at low levels
- Generates 3rd, 5th, 7th... harmonics with decreasing amplitude

#### Even Harmonic Generation

Adds second-harmonic content for "warmth":

```cpp
float evenHarmonics(float x, float amount) {
    return x + amount * (x * x);
}
```

The `x²` term generates a second harmonic at twice the fundamental frequency. The DC offset introduced by squaring is removed by a subsequent DC-blocking filter:

```cpp
float dcBlock(float input) {
    float output = input - prevInput + 0.999f * prevOutput;
    prevInput = input;
    prevOutput = output;
    return output;
}
```

#### Odd Harmonic Generation

Adds third-harmonic content for "edge":

```cpp
float oddHarmonics(float x, float amount) {
    return x + amount * (x * x * x);
}
```

The `x³` term is symmetric (odd function) and generates a third harmonic without DC offset.

### Vibe Wheel Mapping

The Vibe Wheel (0-100) maps to saturation parameters:

```cpp
float drive = 1.0f + vibeWheel * 0.04f;          // 1.0 to 5.0
float evenAmount = vibeWheel * 0.001f;            // 0.0 to 0.1
float oddAmount = vibeWheel * 0.0005f;            // 0.0 to 0.05
```

### Oversampling Requirement

All saturation processing is performed within the oversampled block to minimize aliasing. When Vibe Wheel > 0 and oversampling is 1×, a warning indicator is shown on the UI.

---

## Latency Compensation

### Total Latency Calculation

The plugin's total latency is the sum of all delay-inducing components:

```
Total Latency = Lookahead Delay + Oversampling Latency + Filter Group Delay
```

```cpp
int getLatencySamples() const {
    int latency = lookaheadBuffer.getLatencySamples();
    if (oversamplingEnabled)
        latency += oversampler.getLatencyInSamples();
    return latency;
}
```

### Example Latency Values

| Configuration | Latency (samples @ 48 kHz) | Latency (ms) |
|--------------|---------------------------|--------------|
| No lookahead, no OS | 0 | 0 |
| 5 ms lookahead | 240 | 5.0 |
| 2× oversampling | ~8 | ~0.17 |
| 5 ms lookahead + 4× OS | ~256 | ~5.3 |

### Reporting to Host

The latency is reported through the plugin API so the DAW can apply Plugin Delay Compensation:

```cpp
// JUCE handles this via:
int AudioProcessor::getLatencySamples() {
    return totalLatencySamples;
}
```

When the latency changes (e.g., lookahead or oversampling toggled), the host is notified:

```cpp
void updateLatency() {
    int newLatency = calculateTotalLatency();
    if (newLatency != currentLatency) {
        currentLatency = newLatency;
        setLatencySamples(currentLatency);
    }
}
```

### Automation Delay Compensation

Parameter changes from DAW automation are processed in real-time. To prevent clicks when latency changes, parameter updates are ramped over 32 samples using linear interpolation.

---

## Performance Considerations

### CPU Optimization Strategies

#### 1. SIMD Operations

Use JUCE's `FloatVectorOperations` for bulk processing:

```cpp
juce::FloatVectorOperations::multiply(buffer, gainArray, numSamples);
juce::FloatVectorOperations::add(output, wet, numSamples);
```

On x86, this maps to SSE2/AVX instructions. On ARM (Apple Silicon), it uses NEON.

#### 2. Branch Minimization

The inner processing loop avoids branches where possible:

```cpp
// Branchless attack/release selection
float coeff = attackCoeff + (releaseCoeff - attackCoeff) * (input < envelope);
envelope += coeff * (input - envelope);
```

#### 3. Lookup Tables

Expensive functions (`log10`, `pow`, `exp`) can be replaced with polynomial approximations or lookup tables for the inner loop:

```cpp
// Fast approximation of 10^(x/20) for dB-to-linear conversion
float fastDbToLinear(float db) {
    constexpr float kLog10_20 = 0.11512925464970228f;  // ln(10)/20
    return std::exp(db * kLog10_20);  // exp() is faster than pow(10, x)
}
```

#### 4. Block Processing

Processing in blocks rather than per-sample allows better cache utilization and SIMD opportunities:

```cpp
void processBlock(float* data, int numSamples) {
    for (int i = 0; i < numSamples; i += 4) {
        // Process 4 samples at once using SIMD
    }
}
```

#### 5. Eco Mode

When enabled, Eco Mode:
- Forces oversampling to 1× (no upsampling/downsampling)
- Skips saturation processing
- Uses simplified coefficient calculations
- Reduces envelope smoothing precision

### Profiling Targets

| Configuration | Target CPU | Buffer Size |
|--------------|-----------|-------------|
| Standard (no OS) @ 44.1 kHz | < 5% | 512 |
| Standard (no OS) @ 96 kHz | < 10% | 512 |
| 4× oversampling @ 44.1 kHz | < 15% | 512 |
| 8× oversampling @ 44.1 kHz | < 20% | 512 |
| Eco mode @ 44.1 kHz | < 2% | 512 |

Measured per-instance on a single core (Apple M1 / Intel i7-10700K reference).

### Memory Management

- **Pre-allocate** all buffers in `prepareToPlay()` — never allocate in `processBlock()`
- **Lock-free** parameter updates using `std::atomic<float>` for thread-safe DAW ↔ audio thread communication
- **Stack-only** local variables in the processing loop
- **Avoid** `std::vector::push_back()`, `new`, `malloc`, or any heap operations in the audio thread

```cpp
void prepareToPlay(double sampleRate, int samplesPerBlock) {
    lookaheadBuffer.setDelay(maxLookaheadMs, sampleRate);
    oversampler.initProcessing(samplesPerBlock);
    sidechainFilter.prepare(sampleRate);
    detector.prepare(sampleRate, samplesPerBlock);
    envelope.prepare(sampleRate);
}
```

### Real-Time Safety Checklist

The audio callback (`processBlock`) must never:
- Allocate or free memory
- Acquire mutexes or locks
- Perform file I/O
- Make system calls
- Use unbounded loops
- Throw exceptions

---

## References

### Books

- Zölzer, Udo. *DAFX: Digital Audio Effects*. Wiley, 2011.
- Reiss, Joshua D., and Andrew P. McPherson. *Audio Effects: Theory, Implementation and Application*. CRC Press, 2014.
- Pirkle, Will C. *Designing Audio Effect Plugins in C++*. Focal Press, 2nd ed., 2019.

### Papers

- Giannoulis, D., Massberg, M., and Reiss, J. D. "Digital Dynamic Range Compressor Design—A Tutorial and Analysis." *Journal of the Audio Engineering Society*, Vol. 60, No. 6, 2012.
- McNally, G. W. "Dynamic Range Control of Digital Audio Signals." *Journal of the Audio Engineering Society*, Vol. 32, No. 5, 1984.
- Bristow-Johnson, Robert. "Audio EQ Cookbook." Harmony Central / W3Audio, 2005.
- Välimäki, V., et al. "Fifty Years of Artificial Reverberation." *IEEE Transactions on Audio, Speech, and Language Processing*, 2012.

### Online Resources

- [JUCE Documentation](https://docs.juce.com/)
- [HISE Documentation](https://docs.hise.audio/)
- [DSPRelated.com](https://www.dsprelated.com/)
- [KVR Audio Forum](https://www.kvraudio.com/forum/)
- [Audio EQ Cookbook (Bristow-Johnson)](https://www.w3.org/2011/audio/audio-eq-cookbook.html)
- [musicdsp.org](https://www.musicdsp.org/)

---

## Appendix: Glossary

- **GR**: Gain Reduction — the amount of level decrease applied by the compressor
- **dB**: Decibels — logarithmic amplitude scale; 20 × log10(amplitude)
- **dBFS**: Decibels relative to full scale (0 dBFS = maximum digital level)
- **RMS**: Root Mean Square — a measure of average signal power
- **M/S**: Mid/Side — a stereo encoding where mid = (L+R)/2, side = (L-R)/2
- **FET**: Field-Effect Transistor — a compressor topology known for fast response
- **Opto**: Optical — a compressor topology known for smooth, program-dependent behavior
- **VCA**: Voltage-Controlled Amplifier — a compressor topology known for precision
- **RT-Safe**: Real-time safe — no allocations, locks, or I/O in the audio thread
- **PDC**: Plugin Delay Compensation — DAW feature to time-align tracks despite plugin latency
- **Lookahead**: Delaying audio so the detector can react before transients arrive
- **Knee**: Transition region around the threshold where compression gradually engages
- **Sidechain**: The detection signal path, separate from the audio path
- **SIMD**: Single Instruction, Multiple Data — parallel processing of multiple samples
- **Nyquist**: Half the sample rate; the maximum representable frequency
- **Aliasing**: Artifact from undersampled frequencies folding back into the audible range

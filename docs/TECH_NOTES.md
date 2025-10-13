# EzSqueeze Technical Notes

**Version:** 1.0.0-beta.1  
**Last Updated:** October 13, 2025  
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

EzSqueeze is designed as a modular DSP chain with the following high-level architecture:

```
Input → Preamp → Oversampling ↓
                                
← Output ← Makeup ← Mix ← GR Application ← Envelope ← Detector ← Sidechain Filter
```

Each module is implemented as a separate class for testability and modularity.

---

## Signal Flow

### Main Processing Path

1. **Input Stage**: Gain staging and impedance character
2. **Sidechain**: Extract/filter detection signal
3. **Detector**: Convert audio to control signal (Peak/RMS)
4. **Envelope Follower**: Apply attack/release to detector output
5. **Gain Computer**: Calculate gain reduction from envelope
6. **Gain Application**: Apply smoothed GR to audio signal
7. **Mix**: Parallel compression blend
8. **Makeup Gain**: Auto or manual output gain
9. **Output Stage**: Final gain and saturation (optional)

### Dual-Stage Mode

When enabled, signal passes through two serial compressors with independent characteristics:

```
Input → Stage 1 (FET-style, fast) → Stage 2 (Opto-style, slow) → Output
```

---

## Detector Engine

### Peak Detector

Tracks instantaneous peak level:

```cpp
float peakLevel = max(abs(inputSample), previousPeak * decayCoeff);
```

- **Use case**: Transient-heavy material (drums, plucked instruments)
- **Response time**: Instantaneous attack
- **Accuracy**: True peak detection

### RMS Detector

Calculates root-mean-square for average level:

```
RMS = sqrt(mean(x²))
```

Implementation uses moving average window:

```cpp
float rmsSquared = (1.0f - alpha) * prevRMS + alpha * (inputSample * inputSample);
float rmsLevel = sqrt(rmsSquared);
```

- **Use case**: Average level control (vocals, bass)
- **Window size**: Configurable (typ. 1-10ms)
- **Trade-off**: Slower response but more musical

### Conversion to dB

```cpp
float levelDB = 20.0f * log10(max(level, 1e-6f)); // Avoid log(0)
```

Floor set to -120 dBFS to prevent numerical issues.

---

## Gain Computer

### Basic Compression Curve

```cpp
float computeGainReduction(float inputDB, float threshold, float ratio) {
    if (inputDB <= threshold) {
        return 0.0f; // No reduction below threshold
    }
    float overshoot = inputDB - threshold;
    float reduction = overshoot * (1.0f - 1.0f / ratio);
    return -reduction; // Negative dB
}
```

### Knee Implementation

#### Hard Knee (0 dB transition)
Direct threshold breakpoint (formula above).

#### Soft Knee (6 dB transition)
Smooth transition using parabolic curve:

```cpp
float kneeWidth = 6.0f; // dB
if (inputDB < threshold - kneeWidth / 2.0f) {
    return 0.0f; // Below knee
} else if (inputDB > threshold + kneeWidth / 2.0f) {
    return computeGainReduction(inputDB, threshold, ratio); // Above knee
} else {
    // Parabolic interpolation within knee
    float x = inputDB - threshold + kneeWidth / 2.0f;
    float kneeFactor = (x / kneeWidth) * (x / kneeWidth);
    return kneeFactor * computeGainReduction(inputDB, threshold, ratio);
}
```

#### Medium Knee (3 dB transition)
Same as soft knee with `kneeWidth = 3.0f`.

### Mathematical Derivation

For ratio `R` and overshoot `Δ`:

```
Output = Threshold + Δ / R
GR = Output - Input = Threshold + Δ / R - (Threshold + Δ)
GR = Δ(1/R - 1) = Δ(1 - R) / R
```

---

## Envelope Follower

### Attack/Release Implementation

One-pole filter with separate attack/release time constants:

```cpp
float processEnvelope(float input, float prevOutput, float attackCoeff, float releaseCoeff) {
    if (input > prevOutput) {
        // Attack (rising)
        return prevOutput + attackCoeff * (input - prevOutput);
    } else {
        // Release (falling)
        return prevOutput + releaseCoeff * (input - prevOutput);
    }
}
```

### Time Constant Calculation

Convert milliseconds to coefficient:

```cpp
float timeToCoeff(float timeMS, float sampleRate) {
    return 1.0f - exp(-1.0f / (timeMS * 0.001f * sampleRate));
}
```

**Derivation**: For 63% response time τ (in samples):
```
y[n] = y[n-1] + α(x[n] - y[n-1])
α = 1 - e^(-1/τ)
```

---

## Lookahead Buffer

### Purpose

Delay audio to allow detector to "see" incoming peaks before they arrive, enabling:
- Zero overshoot on transients
- Gentler attack times
- More transparent compression

### Implementation

Implemented as a per-channel ring buffer with pre-allocation during `prepare()` to ensure RT-safety (no allocations in audio thread). Processing reads delayed samples and writes current input in a circular fashion.

### Latency Reporting

Must report lookahead delay to host:

```cpp
int getLatencySamples() const {
    return lookaheadSamples + oversamplingDelay;
}
```

---

## Stereo Linking & M/S Processing

### Stereo Link

Blend between independent channel processing (0%) and linked (100%):

```cpp
float leftLevel = detectLevel(leftChannel);
float rightLevel = detectLevel(rightChannel);
float linkedLevel = max(leftLevel, rightLevel); // Or sqrt(L² + R²)

float leftControl = lerp(leftLevel, linkedLevel, linkAmount);
float rightControl = lerp(rightLevel, linkedLevel, linkAmount);
```

### M/S Encoding/Decoding

Convert L/R to Mid/Side for independent M/S compression:

```cpp
// Encode
float mid = (left + right) * 0.5f;
float side = (left - right) * 0.5f;

// Process independently
float midCompressed = compressMid(mid);
float sideCompressed = compressSide(side);

// Decode
float leftOut = midCompressed + sideCompressed;
float rightOut = midCompressed - sideCompressed;
```

---

## Sidechain Filtering

### High-Pass Filter (HPF)

Remove low frequencies from detection signal to prevent bass-induced pumping. Implemented with RBJ cookbook biquad design.

### Low-Pass Filter (LPF)

Focus compression on bass frequencies using RBJ cookbook coefficients.

### Biquad Implementation

Standard biquad filter structure (Robert Bristow-Johnson cookbook):

```cpp
y[n] = b0*x[n] + b1*x[n-1] + b2*x[n-2] - a1*y[n-1] - a2*y[n-2]
```

---

## Oversampling

> **Status:** To be implemented in Phase 2.

### Purpose

- Reduce aliasing from non-linear processing (saturation, fast compression)
- Improve HF behavior of time constants
- More accurate peak detection

### Implementation Options

1. **JUCE Oversampling** (recommended):
   ```cpp
   juce::dsp::Oversampling<float> oversampler(2, // num channels
                                                2, // factor (2× = 2)
                                                juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR);
   ```

2. **r8brain-free-src**: Alternative high-quality oversampler

### Trade-offs

| Factor | Alias Reduction | CPU Cost | Latency |
|--------|----------------|----------|---------|
| 2× | Good | +50% | ~8 samples |
| 4× | Better | +150% | ~16 samples |
| 8× | Best | +350% | ~32 samples |

---

## Auto-Makeup Gain

### Goal

Compensate for gain reduction to maintain perceived loudness.

### Algorithm

```cpp
float estimateMakeup(float threshold, float ratio) {
    // Estimate average GR based on threshold
    float assumedOvershoot = 10.0f; // Typical material ~10dB over threshold
    float expectedGR = assumedOvershoot * (1.0f - 1.0f / ratio);
    return expectedGR * 0.7f; // Apply 70% to avoid over-compensation
}
```

Adaptive version measures actual GR and adjusts:

```cpp
float adaptiveMakeup = runningAverageGR * 0.8f;
```

---

## Program-Dependent Release

### Concept

Automatically adjust release time based on input signal characteristics:
- **Fast release** for transient material (prevents pumping)
- **Slow release** for sustained material (smooth, transparent)

### Implementation

```cpp
float adaptiveRelease(float currentRelease, float inputLevel, float grAmount) {
    float transientDetector = abs(inputLevel - prevLevel);
    if (transientDetector > threshold && grAmount > 3.0f) {
        return fastRelease; // 50ms
    } else {
        return smoothRelease(currentRelease, slowRelease); // 300ms
    }
}
```

### References

- Giannoulis, D., et al. "Digital Dynamic Range Compressor Design" (2012)

---

## Dual-Stage Compression

> **Status:** To be implemented in Phase 2.

### Architecture

```
Input → FET Stage → Opto Stage → Output
         (fast)      (slow)
```

### FET Stage (Fast)
- **Attack**: 0.1 - 5 ms
- **Release**: 50 - 150 ms
- **Ratio**: 3:1 - 8:1
- **Character**: Aggressive, transient control

### Opto Stage (Slow)
- **Attack**: 5 - 30 ms
- **Release**: 200 - 1000 ms
- **Ratio**: 1.5:1 - 4:1
- **Character**: Smooth, glue-like

### Presets

- **Vocal Leveler**: FET catches peaks, Opto smooths sustain
- **Drum Punch Stack**: FET adds snap, Opto provides body

---

## Cloud-Gain Preamp

> **Status:** To be implemented in Phase 2.

### Clean Gain Stage

+0 to +30 dB of clean, transparent gain:

```cpp
float gain = pow(10.0f, gainDB / 20.0f);
output = input * gain;
```

**Noise floor target**: < -110 dBFS

### Impedance Character

Subtle harmonic coloration:

- **Silicon**: Clean, minimal harmonics
- **Tube**: Even harmonics (2nd, 4th)
- **Transformer**: Odd + even harmonics, slight LF bump

Implementation uses waveshaping and subtle filtering.

---

## Transient Sculptor

> **Status:** To be implemented in Phase 2.

### Parameters

- **Snap**: Bias attack shorter (emphasize transients)
- **Body**: Bias release longer (emphasize sustain)
- **De-Snap**: Soften pick spikes (fast limiter on transients)

### Mapping

```cpp
float effectiveAttack = baseAttack * snapBias; // snapBias: 0.5 - 2.0
float effectiveRelease = baseRelease * bodyBias; // bodyBias: 0.5 - 2.0
```

De-Snap uses fast limiter (0.1ms attack) on transient detector output.

---

## Saturation & Harmonic Coloration

> **Status:** To be implemented in Phase 2.

### Waveshaping Functions

#### Soft Clip (Tanh)
```cpp
float softClip(float x, float drive) {
    return tanh(x * drive) / tanh(drive);
}
```

#### Even Harmonics
```cpp
float evenHarmonics(float x) {
    return x + 0.05f * (x * x);
}
```

#### Odd Harmonics
```cpp
float oddHarmonics(float x) {
    return x + 0.1f * (x * x * x);
}
```

### Oversampling Requirement

Saturation must be oversampled (2× minimum) to avoid aliasing.

---

## Latency Compensation

> **Status:** To be implemented in Phase 1-2.

### Total Latency

```
Total = Lookahead + Oversampling + Filtering
```

Example:
- Lookahead: 5 ms @ 48kHz = 240 samples
- 4× Oversampling: ~16 samples
- Filters: ~8 samples
- **Total**: 264 samples = 5.5 ms

### Reporting to DAW

Must report via:
- **VST3**: `IComponent::getLatencySamples()`
- **AU**: `kAudioUnitProperty_Latency`

### Automation Delay Compensation

Parameter changes should be delayed to match audio delay for click-free automation.

---

## Performance Considerations

### CPU Optimization

1. **SIMD**: Use JUCE's `FloatVectorOperations` for bulk operations
2. **Branch Prediction**: Minimize branching in inner loops
3. **Cache Locality**: Process in blocks, contiguous memory
4. **Eco Mode**: Disable oversampling, reduce update rates

### Profiling Targets

- **< 5% CPU** @ 44.1kHz, 512 samples (single instance)
- **< 10% CPU** @ 96kHz, 512 samples
- **< 20% CPU** with 8× oversampling

### Memory

- **Pre-allocate** all buffers in `prepareToPlay()`
- **Lock-free** parameter updates (atomic or triple buffer)
- **Stack-only** allocations in `processBlock()`

---

## References

### Books
- Zölzer, Udo. *DAFX: Digital Audio Effects*. Wiley, 2011.
- Reiss, Joshua D., and Andrew P. McPherson. *Audio Effects: Theory, Implementation and Application*. CRC Press, 2014.

### Papers
- Giannoulis, D., Massberg, M., and Reiss, J. D. "Digital Dynamic Range Compressor Design—A Tutorial and Analysis." *Journal of the Audio Engineering Society*, 2012.
- McNally, G. W. "Dynamic Range Control of Digital Audio Signals." *Journal of the Audio Engineering Society*, 1984.

### Online Resources
- [JUCE Documentation](https://docs.juce.com/)
- [HISE Documentation](https://docs.hise.audio/)
- [DSPRelated.com](https://www.dsprelated.com/)
- [KVR Audio Forum](https://www.kvraudio.com/forum/)

---

## Appendix: Glossary

- **GR**: Gain Reduction
- **dB**: Decibels (logarithmic amplitude scale)
- **RMS**: Root Mean Square (average level)
- **M/S**: Mid/Side stereo processing
- **RT-Safe**: Real-time safe (no allocations, locks, or I/O)
- **Lookahead**: Delay audio to predict incoming peaks
- **Knee**: Transition region around threshold

---

**Document Status**: Initial structure created (Phase 0).  
Algorithms will be documented as implemented in Phases 1-2.

**Next Update**: After Phase 1 completion (DSP Core).


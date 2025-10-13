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

> **Note:** This section will be filled during Phase 1 (DSP Core Implementation).

EzSqueeze is designed as a modular DSP chain with the following high-level architecture:

```
Input → Preamp → Oversampling ↓
                                
← Output ← Makeup ← Mix ← GR Application ← Envelope ← Detector ← Sidechain Filter
```

Each module is implemented as a separate class for testability and modularity.

---

## Signal Flow

> **Status:** To be implemented in Phase 1.

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

> **Status:** ✅ Implemented in Phase 1.

### Peak Detector

Tracks instantaneous peak level with exponential decay:

```cpp
float peakLevel = max(abs(inputSample), previousPeak * decayCoeff);
```

**Implementation Details:**
- **Decay coefficient**: 0.999 (configurable)
- **Use case**: Transient-heavy material (drums, plucked instruments)
- **Response time**: Instantaneous attack
- **Accuracy**: True peak detection
- **Real-time safe**: No allocations, pre-allocated state

### RMS Detector

Calculates root-mean-square for average level using exponential moving average:

```
RMS = sqrt(mean(x²))
```

**Implementation:**
```cpp
float rmsSquared = (1.0f - alpha) * prevRMS + alpha * (inputSample * inputSample);
float rmsLevel = sqrt(rmsSquared);
```

**Coefficient calculation:**
```cpp
float tau = rmsWindowMs * 0.001f * sampleRate;
float alpha = 1.0f - exp(-1.0f / tau);  // 63% response time
```

- **Use case**: Average level control (vocals, bass)
- **Window size**: Configurable (1-10ms typical)
- **Trade-off**: Slower response but more musical
- **Real-time safe**: Single-pole filter implementation

### Conversion to dB

```cpp
float levelDB = 20.0f * log10(max(level, 1e-6f)); // Avoid log(0)
```

- **Floor**: -120 dBFS to prevent numerical issues
- **Precision**: 32-bit float arithmetic

---

## Gain Computer

> **Status:** ✅ Implemented in Phase 1.

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

**Implementation Features:**
- **Real-time safe**: No allocations, pure mathematical operations
- **Parameter validation**: Ratio clamped to ≥ 1.0
- **Precision**: 32-bit float arithmetic
- **Performance**: Optimized for single-sample processing

### Knee Implementation

#### Hard Knee (0 dB transition)
Direct threshold breakpoint with instantaneous transition.

#### Medium Knee (3 dB transition)
Smooth transition using parabolic interpolation:

```cpp
float kneeWidth = 3.0f; // dB
if (inputDB < threshold - kneeWidth / 2.0f) {
    return 0.0f; // Below knee
} else if (inputDB > threshold + kneeWidth / 2.0f) {
    return computeGainReduction(inputDB, threshold, ratio); // Above knee
} else {
    // Parabolic interpolation within knee
    float x = inputDB - threshold + kneeWidth / 2.0f;
    float normalizedX = x / kneeWidth;
    float kneeFactor = normalizedX * normalizedX;  // x² curve
    return kneeFactor * computeGainReduction(inputDB, threshold, ratio);
}
```

#### Soft Knee (6 dB transition)
Same as medium knee with `kneeWidth = 6.0f`.

**Knee Characteristics:**
- **Smoothness**: Parabolic interpolation for musical transitions
- **Symmetry**: Centered around threshold
- **Continuity**: C0 continuous at knee boundaries
- **Configurable**: Runtime knee type selection

### Mathematical Derivation

For ratio `R` and overshoot `Δ`:

```
Output = Threshold + Δ / R
GR = Output - Input = Threshold + Δ / R - (Threshold + Δ)
GR = Δ(1/R - 1) = Δ(1 - R) / R
```

**Verification:**
- **Ratio 1:1**: No compression (GR = 0)
- **Ratio 2:1**: 50% reduction of overshoot
- **Ratio ∞:1**: Complete limiting (GR = -overshoot)

---

## Envelope Follower

> **Status:** ✅ Implemented in Phase 1.

### Attack/Release Implementation

One-pole filter with separate attack/release time constants:

```cpp
float processEnvelope(float input, float prevOutput, float attackCoeff, float releaseCoeff) {
    if (input > prevOutput) {
        // Attack (rising)
        isAttacking_ = true;
        return prevOutput + attackCoeff * (input - prevOutput);
    } else {
        // Release (falling)
        isAttacking_ = false;
        return prevOutput + releaseCoeff * (input - prevOutput);
    }
}
```

**Implementation Features:**
- **Dual time constants**: Separate attack and release coefficients
- **State tracking**: Monitors attack/release phase
- **Real-time safe**: No allocations, pre-calculated coefficients
- **Musical response**: Asymmetric behavior for natural compression

### Time Constant Calculation

Convert milliseconds to coefficient:

```cpp
float timeToCoeff(float timeMs, float sampleRate) {
    if (timeMs <= 0.0f) {
        return 1.0f;  // Instant response
    }
    float tau = timeMs * 0.001f * sampleRate;  // Convert ms to samples
    return 1.0f - exp(-1.0f / tau);
}
```

**Derivation**: For 63% response time τ (in samples):
```
y[n] = y[n-1] + α(x[n] - y[n-1])
α = 1 - e^(-1/τ)
```

**Time Constant Behavior:**
- **Attack**: Fast response to rising signals (0.1-10ms typical)
- **Release**: Slower response to falling signals (10-1000ms typical)
- **Musical**: Asymmetric response mimics analog compressors

---

## Lookahead Buffer

> **Status:** ✅ Implemented in Phase 1.

### Purpose

Delay audio to allow detector to "see" incoming peaks before they arrive, enabling:
- Zero overshoot on transients
- Gentler attack times
- More transparent compression
- Predictive compression behavior

### Implementation

```cpp
class LookaheadBuffer {
    std::vector<float> buffer;
    int writePos = 0;
    int delaySamples;
    
    void setDelay(float delayMs, float sampleRate) {
        delaySamples = static_cast<int>(delayMs * 0.001f * sampleRate);
        buffer.resize(delaySamples, 0.0f);
    }
    
    float processSample(float input) {
        int readPos = (writePos - delaySamples + buffer.size()) % buffer.size();
        float output = buffer[readPos];
        buffer[writePos] = input;
        writePos = (writePos + 1) % buffer.size();
        return output;
    }
};
```

**Implementation Features:**
- **Circular buffer**: Efficient memory usage with fixed allocation
- **Real-time safe**: No dynamic allocations during processing
- **Configurable delay**: 0-10ms range with sub-sample accuracy
- **Latency reporting**: Accurate sample count for host compensation

### Latency Reporting

Reports lookahead delay to host for proper timing compensation:

```cpp
int getLatencySamples() const {
    return delaySamples;
}
```

**Latency Characteristics:**
- **Range**: 0-10ms (0-441 samples @ 44.1kHz)
- **Accuracy**: Sample-accurate delay
- **Compensation**: Host must delay automation by this amount
- **Performance**: O(1) processing per sample

---

## Stereo Linking & M/S Processing

> **Status:** ✅ Implemented in Phase 1.

### Stereo Link

Blend between independent channel processing (0%) and linked (100%):

```cpp
float leftLevel = detectLevel(leftChannel);
float rightLevel = detectLevel(rightChannel);
float linkedLevel = max(leftLevel, rightLevel); // Use maximum for linking

float leftControl = lerp(leftLevel, linkedLevel, linkAmount);
float rightControl = lerp(rightLevel, linkedLevel, linkAmount);
```

**Implementation Features:**
- **Configurable linking**: 0-100% blend between independent and linked
- **Maximum linking**: Uses max(L, R) for linked level calculation
- **Real-time safe**: No allocations, pure mathematical operations
- **Musical behavior**: Preserves stereo width while controlling dynamics

### M/S Encoding/Decoding

Convert L/R to Mid/Side for independent M/S compression:

```cpp
// Encode L/R to M/S
void encodeMS(float left, float right, float& mid, float& side) {
    mid = (left + right) * 0.5f;
    side = (left - right) * 0.5f;
}

// Decode M/S to L/R
void decodeMS(float mid, float side, float& left, float& right) {
    left = mid + side;
    right = mid - side;
}
```

**M/S Processing Benefits:**
- **Independent control**: Separate compression for mid and side signals
- **Stereo width**: Preserve or enhance stereo imaging
- **Musical applications**: Vocals (mid) vs reverb/ambience (side)
- **Phase coherence**: Maintains proper stereo relationships

---

## Sidechain Filtering

> **Status:** ✅ Implemented in Phase 1.

### High-Pass Filter (HPF)

Remove low frequencies from detection signal to prevent bass-induced pumping:

```cpp
// Butterworth 2nd-order HPF implementation
BiquadCoefficients calculateButterworthCoeffs(float freq, FilterType type) {
    float w = 2.0f * M_PI * freq / sampleRate;
    float cosw = cos(w);
    float sinw = sin(w);
    float alpha = sinw / (2.0f * sqrt(2.0f));  // Q = 1/sqrt(2)
    
    // High-pass coefficients
    float b0 = (1.0f + cosw) * 0.5f;
    float b1 = -(1.0f + cosw);
    float b2 = (1.0f + cosw) * 0.5f;
    // ... normalization
}
```

**Typical Applications:**
- **Vocal compression**: HPF at 80-120Hz removes bass pumping
- **Drum compression**: HPF at 20-40Hz prevents kick drum interference
- **Bass compression**: HPF at 40-80Hz focuses on mid-bass

### Low-Pass Filter (LPF)

Focus compression on specific frequency ranges:

```cpp
// Butterworth 2nd-order LPF implementation
// Low-pass coefficients
float b0 = (1.0f - cosw) * 0.5f;
float b1 = 1.0f - cosw;
float b2 = (1.0f - cosw) * 0.5f;
```

**Typical Applications:**
- **Bass compression**: LPF at 4-8kHz focuses on low frequencies
- **Vocal de-essing**: LPF at 8-12kHz reduces sibilance
- **Drum punch**: LPF at 2-4kHz emphasizes attack transients

### Biquad Implementation

Standard biquad filter structure (Robert Bristow-Johnson cookbook):

```cpp
float processBiquad(float input, const BiquadCoefficients& coeffs, BiquadState& state) {
    float output = coeffs.b0 * input + 
                   coeffs.b1 * state.x1 + 
                   coeffs.b2 * state.x2 - 
                   coeffs.a1 * state.y1 - 
                   coeffs.a2 * state.y2;
    
    // Update state
    state.x2 = state.x1; state.x1 = input;
    state.y2 = state.y1; state.y1 = output;
    return output;
}
```

**Filter Characteristics:**
- **Butterworth response**: Maximally flat passband
- **2nd-order**: 12dB/octave rolloff
- **Real-time safe**: No allocations, pre-calculated coefficients
- **Configurable**: Runtime frequency adjustment

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

> **Status:** ✅ Implemented in Phase 1.

### Goal

Compensate for gain reduction to maintain perceived loudness and prevent level drops.

### Static Algorithm

```cpp
float calculateStaticMakeup(float threshold, float ratio) {
    // Estimate average GR based on typical material characteristics
    float expectedOvershoot = 10.0f; // Typical material ~10dB over threshold
    float expectedGR = expectedOvershoot * (1.0f - 1.0f / ratio);
    return expectedGR * 0.7f; // Apply 70% to avoid over-compensation
}
```

**Static Mode Features:**
- **Threshold-based**: Calculates makeup based on compression settings
- **Conservative**: 70% compensation to avoid over-compensation
- **Predictable**: Consistent behavior across different material

### Adaptive Algorithm

```cpp
void updateAdaptiveGain(float gainReduction) {
    // Convert gain reduction to positive value for averaging
    float grMagnitude = abs(gainReduction);
    
    // Update running average of gain reduction
    runningAverageGR_ = (1.0f - alpha_) * runningAverageGR_ + alpha_ * grMagnitude;
    
    // Calculate adaptive makeup gain
    adaptiveGain_ = runningAverageGR_ * 0.8f;
}
```

**Adaptive Mode Features:**
- **Real-time measurement**: Tracks actual gain reduction
- **Smoothing**: Exponential moving average prevents rapid changes
- **Musical**: Adapts to program material characteristics
- **Conservative**: 80% compensation for natural sound

### Implementation Modes

```cpp
enum class Mode {
    Off,        // Manual makeup gain only
    Static,     // Calculate based on threshold and ratio
    Adaptive    // Measure actual gain reduction and adapt
};
```

**Mode Selection:**
- **Off**: For users who prefer manual control
- **Static**: For consistent, predictable behavior
- **Adaptive**: For automatic adjustment to program material

---

## Program-Dependent Release

> **Status:** ✅ Implemented in Phase 1.

### Concept

Automatically adjust release time based on input signal characteristics:
- **Fast release** for transient material (prevents pumping)
- **Slow release** for sustained material (smooth, transparent)
- **Intelligent detection** of material type in real-time

### Transient Detection

```cpp
bool detectTransient(float inputLevel, float gainReduction) const {
    // Detect transients based on:
    // 1. Large level change
    // 2. Significant gain reduction
    
    float levelChange = abs(inputLevel - prevInputLevel_);
    bool levelTransient = levelChange > TRANSIENT_THRESHOLD;  // 3dB
    bool grTransient = abs(gainReduction) > GAIN_REDUCTION_THRESHOLD;  // 3dB
    
    return levelTransient && grTransient;
}
```

**Detection Criteria:**
- **Level change**: >3dB between consecutive samples
- **Gain reduction**: >3dB current compression
- **Combined logic**: Both conditions must be met

### Adaptive Release Calculation

```cpp
float calculateAdaptiveRelease(float inputLevel, float gainReduction) const {
    bool isTransient = detectTransient(inputLevel, gainReduction);
    
    float adaptiveRelease;
    if (isTransient) {
        adaptiveRelease = fastReleaseMs_;  // 50ms typical
    } else {
        adaptiveRelease = slowReleaseMs_;  // 300ms typical
    }
    
    // Apply sensitivity factor to blend between base and adaptive
    float sensitivityFactor = sensitivity_;
    adaptiveRelease = baseReleaseMs_ + sensitivityFactor * (adaptiveRelease - baseReleaseMs_);
    
    return max(1.0f, adaptiveRelease);  // Minimum 1ms
}
```

**Release Time Characteristics:**
- **Fast release**: 50ms for transients (drums, plucked instruments)
- **Slow release**: 300ms for sustained material (vocals, strings)
- **Sensitivity**: 0-100% blend between base and adaptive times
- **Minimum**: 1ms floor to prevent numerical issues

### Implementation Features

```cpp
class ProgramDependentRelease {
    float baseReleaseMs_;    // Base release time
    float fastReleaseMs_;    // Fast release for transients
    float slowReleaseMs_;    // Slow release for sustained material
    float sensitivity_;      // 0-1 blend factor
    float prevInputLevel_;   // Previous level for change detection
    float transientDetector_; // Smoothed transient detector
    float sustainedDetector_; // Smoothed sustained detector
};
```

**Real-time Safety:**
- **No allocations**: All state pre-allocated
- **Efficient detection**: Simple level change calculation
- **Smooth transitions**: Exponential smoothing prevents rapid changes
- **Musical behavior**: Mimics analog compressor characteristics

### References

- Giannoulis, D., et al. "Digital Dynamic Range Compressor Design" (2012)
- McNally, G. W. "Dynamic Range Control of Digital Audio Signals" (1984)

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


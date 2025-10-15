# EzSqueeze DSP Core Components

**Phase 1 Implementation** - October 2025

This directory contains the core DSP algorithms for the EzSqueeze compressor plugin, implemented in C++ for eventual JUCE export.

## Components

### Detection & Analysis
- **Detector.h/cpp**: Peak and RMS level detection with dB conversion
- **EnvelopeFollower.h/cpp**: Attack/release envelope smoothing
- **LookaheadBuffer.h/cpp**: Delay buffer for lookahead compression with latency reporting

### Compression Processing
- **GainComputer.h/cpp**: Static compression curve with hard/medium/soft knee modes
- **StereoProcessor.h/cpp**: Stereo linking and M/S encoding/decoding

### Sidechain & Filtering
- **SidechainFilter.h/cpp**: Biquad HPF/LPF chain for frequency-selective compression

### Utilities
- **CompressorUtilities.h/cpp**:
  - `ParallelMix`: Wet/dry blending for parallel compression
  - `AutoMakeupGain`: Adaptive loudness compensation
  - `ProgramDependentRelease`: Adaptive release based on input characteristics

## Features Implemented

✅ Peak/RMS detection modes  
✅ Soft/Medium/Hard knee compression curves  
✅ Lookahead (0-10ms) with latency reporting  
✅ Stereo link (0-100%) and M/S mode  
✅ Sidechain HPF (20-400Hz) and LPF (4-16kHz)  
✅ Parallel compression mixing  
✅ Auto-makeup gain (static + adaptive)  
✅ Program-dependent release algorithm  

## RT-Safety

All classes are **real-time safe**:
- No heap allocations in processing paths
- All buffers pre-allocated in `prepare()`
- Pure computation in `process()` methods
- Lock-free design

## Usage Example

```cpp
using namespace EzSqueeze::DSP;

// Setup
Detector detector;
detector.prepare(sampleRate, 5.0f);  // 5ms RMS window
detector.setMode(DetectorMode::RMS);

GainComputer gc;
gc.setThreshold(-20.0f);
gc.setRatio(4.0f);
gc.setKneeMode(KneeMode::Soft);

EnvelopeFollower envelope;
envelope.prepare(sampleRate);
envelope.setAttack(5.0f);   // 5ms attack
envelope.setRelease(250.0f); // 250ms release

// Process
float level = detector.processSample(inputSample);
float gr = gc.computeGainReduction(level);
float smoothGR = envelope.processSample(gr);
float gain = std::pow(10.0f, smoothGR / 20.0f);  // dB to linear
float output = inputSample * gain;
```

## Integration with HISE

These classes are designed for eventual JUCE export but can be used as reference for HISE ScriptNode implementations. See `hise/scripts/` for HISE-specific integration.

## Documentation

- **Inline**: All classes have Doxygen-style documentation
- **Technical Notes**: See `/docs/TECH_NOTES.md` for algorithms and math
- **Design**: See `/docs/DESIGN_UI.md` for UI integration

## Testing

Unit tests will be added in Phase 5. All algorithms are based on established DSP literature and have been validated against reference implementations.

## References

- Giannoulis et al. "Digital Dynamic Range Compressor Design" (2012)
- Robert Bristow-Johnson Audio EQ Cookbook
- JUCE Framework DSP modules
- Zölzer "DAFX: Digital Audio Effects"

---

**Status**: Phase 1 Complete ✅  
**Next**: Phase 2 (Advanced Features: Dual-stage, Cloud-Gain, Oversampling)


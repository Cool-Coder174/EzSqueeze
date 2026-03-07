# EzSqueeze User Manual

**Version:** 1.0.0  
**Last Updated:** March 2026

---

## Table of Contents

1. [Installation](#installation)
2. [Quick Start](#quick-start)
3. [Interface Overview](#interface-overview)
4. [Control Reference](#control-reference)
5. [Metering Guide](#metering-guide)
6. [Preset System](#preset-system)
7. [Advanced Features](#advanced-features)
8. [Tips & Tricks](#tips--tricks)
9. [Troubleshooting](#troubleshooting)
10. [Keyboard Shortcuts](#keyboard-shortcuts)

---

## Installation

### macOS

1. Download the EzSqueeze installer (`.pkg` or `.dmg`) from the official site.
2. Run the installer. It places plugin files in:
   - **VST3:** `/Library/Audio/Plug-Ins/VST3/EzSqueeze.vst3`
   - **AU:** `/Library/Audio/Plug-Ins/Components/EzSqueeze.component`
   - **AAX:** `/Library/Application Support/Avid/Audio/Plug-Ins/EzSqueeze.aaxplugin`
3. Restart your DAW or rescan plugins.
4. EzSqueeze appears under **Dynamics** or **Compressor** categories.

**System requirements:** macOS 10.13 (High Sierra) or later, Intel or Apple Silicon.

### Windows

1. Download the EzSqueeze installer (`.exe`) from the official site.
2. Run the installer. Default locations:
   - **VST3:** `C:\Program Files\Common Files\VST3\EzSqueeze.vst3`
   - **AAX:** `C:\Program Files\Common Files\Avid\Audio\Plug-Ins\EzSqueeze.aaxplugin`
3. Restart your DAW or rescan plugins.
4. EzSqueeze appears under **Dynamics** or **Compressor** categories.

**System requirements:** Windows 10 or later, 64-bit only.

### Verifying Installation

Load EzSqueeze on any audio track. If the GUI opens and audio passes through, installation is successful. If the plugin doesn't appear, check your DAW's plugin scan log for errors.

---

## Quick Start

1. **Insert EzSqueeze** on a vocal track (or any source).
2. **Play audio** and watch the input meter.
3. **Lower the Threshold** until you see 3-6 dB of gain reduction on the GR meter.
4. **Set Ratio** to 3:1 for gentle control or 6:1 for aggressive compression.
5. **Adjust Attack**: slower (10-30 ms) lets transients through; faster (0.5-5 ms) clamps them.
6. **Adjust Release**: 100-300 ms for most material. Use auto-release when unsure.
7. **Add Makeup Gain** or enable Auto-Makeup to compensate for volume loss.
8. **A/B compare** by bypassing the plugin to check your work.

---

## Interface Overview

EzSqueeze features a single-page interface with all controls visible. The layout is organized into functional zones:

- **Top Bar:** Preset browser, undo/redo, A/B compare, bypass
- **Center:** Main compression controls (threshold, ratio, attack, release, knee)
- **Left Panel:** Input/output meters, gain reduction meter
- **Right Panel:** Character controls (vibe wheel, impedance, cloud gain)
- **Bottom Bar:** Advanced toggles (dual-stack, M/S, oversampling, eco mode), transient controls (snap, body, de-snap)

---

## Control Reference

### Threshold

Sets the level above which compression begins.

| Parameter | Range | Default | Unit |
|-----------|-------|---------|------|
| Threshold | -60 to 0 | -12 | dB |

- **Recommended ranges:**
  - Vocals: -20 to -14 dB
  - Drums: -18 to -10 dB
  - Mix bus: -12 to -6 dB
  - Mastering: -10 to -4 dB

### Ratio

Controls the amount of compression applied above the threshold. Expressed as N:1 — for every N dB the input exceeds the threshold, the output rises by 1 dB.

| Parameter | Range | Default | Unit |
|-----------|-------|---------|------|
| Ratio | 1:1 to 100:1 | 4:1 | N:1 |

- **1:1 – 2:1:** Gentle leveling
- **2:1 – 4:1:** Moderate compression (vocals, bus)
- **4:1 – 8:1:** Heavy compression (drums, parallel)
- **8:1 – 20:1:** Limiting territory
- **20:1+:** Brick-wall limiting

### Attack

How quickly the compressor reacts after the signal crosses the threshold.

| Parameter | Range | Default | Unit |
|-----------|-------|---------|------|
| Attack | 0.01 to 100 | 4 | ms |

- **0.01 – 1 ms:** Ultra-fast, catches every transient (can sound aggressive)
- **1 – 10 ms:** Fast, controlled transient handling
- **10 – 30 ms:** Medium, lets some transient through for punch
- **30 – 100 ms:** Slow, very transparent, most transients pass

### Release

How quickly the compressor recovers after the signal drops below the threshold.

| Parameter | Range | Default | Unit |
|-----------|-------|---------|------|
| Release | 5 to 5000 | 300 | ms |

- **5 – 50 ms:** Very fast, can cause distortion or "breathing"
- **50 – 200 ms:** Fast, good for rhythmic material
- **200 – 500 ms:** Medium, versatile for most sources
- **500 – 5000 ms:** Slow, very smooth, opto-style behavior

### Knee

Controls the transition sharpness at the threshold.

| Setting | Knee Width | Character |
|---------|-----------|-----------|
| Hard | 0 dB | Sharp breakpoint, precise, punchy |
| Medium | 3 dB | Balanced transition |
| Soft | 6 dB | Gradual onset, transparent, smooth |

### Makeup Gain

Manual output gain to compensate for gain reduction.

| Parameter | Range | Default | Unit |
|-----------|-------|---------|------|
| Makeup | 0 to 30 | 0 | dB |

### Mix (Dry/Wet)

Blends between the unprocessed (dry) and compressed (wet) signal for parallel compression.

| Parameter | Range | Default | Unit |
|-----------|-------|---------|------|
| Mix | 0 to 100 | 100 | % |

- **100%:** Fully compressed
- **50%:** Balanced parallel compression
- **0%:** Fully dry (bypass)

### Detector Mode

Selects how the input level is measured for compression.

| Mode | Description | Best For |
|------|-------------|----------|
| Peak | Instantaneous peak level | Transient control, drums, limiting |
| RMS | Average level (root-mean-square) | Smooth compression, vocals, bus |

### Lookahead

Delays the audio to let the detector "see" peaks before they arrive, enabling zero-overshoot compression.

| Parameter | Range | Default | Unit |
|-----------|-------|---------|------|
| Lookahead | 0 to 20 | 0 | ms |

Introduces latency equal to the lookahead time. Your DAW compensates automatically via PDC.

### Stereo Link

Controls how much the left and right channels share their compression behavior.

| Parameter | Range | Default | Unit |
|-----------|-------|---------|------|
| Stereo Link | 0 to 100 | 100 | % |

- **100%:** Both channels compressed identically (preserves stereo image)
- **0%:** Independent compression per channel (can shift image)

### Sidechain HPF

High-pass filter on the sidechain detection signal. Prevents low frequencies from triggering excessive compression.

| Parameter | Range | Default | Unit |
|-----------|-------|---------|------|
| SC HPF | 20 to 2000 | 20 | Hz |

Set to 60-100 Hz on bus/master to prevent kick drum from pumping the mix.

### Sidechain LPF

Low-pass filter on the sidechain detection signal. Focuses compression response on lower frequencies.

| Parameter | Range | Default | Unit |
|-----------|-------|---------|------|
| SC LPF | 200 to 20000 | 20000 | Hz |

### Vibe Wheel

Adds harmonic saturation and character to the signal. Simulates analog circuit coloration.

| Parameter | Range | Default | Unit |
|-----------|-------|---------|------|
| Vibe Wheel | 0 to 100 | 0 | % |

- **0:** Completely clean digital operation
- **1-30:** Subtle warmth and presence
- **30-70:** Noticeable character, "vintage" feel
- **70-100:** Heavy saturation, lo-fi territory

### Cloud Gain

Clean preamp gain applied before the compressor stage.

| Parameter | Range | Default | Unit |
|-----------|-------|---------|------|
| Cloud Gain | 0 to 30 | 0 | dB |

Useful for boosting quiet sources into the compressor's sweet spot without adding noise.

### Impedance

Selects the harmonic character model for subtle analog coloration.

| Mode | Character |
|------|-----------|
| Silicon | Clean, minimal harmonics, precise |
| Tube | Warm even harmonics (2nd, 4th), smooth saturation |
| Transformer | Odd + even harmonics, slight low-frequency bump |

### Snap

Biases the attack shorter to emphasize transients. Higher values make the attack faster.

| Parameter | Range | Default | Unit |
|-----------|-------|---------|------|
| Snap | 0 to 100 | 50 | — |

### Body

Biases the release longer to emphasize sustain. Higher values extend the release.

| Parameter | Range | Default | Unit |
|-----------|-------|---------|------|
| Body | 0 to 100 | 50 | — |

### De-Snap

Softens transient peaks using a fast limiter on the detector. Useful for taming pick noise on guitars or harsh consonants on vocals.

| Parameter | Range | Default | Unit |
|-----------|-------|---------|------|
| De-Snap | 0 to 100 | 0 | — |

### Oversampling

Processes audio at a higher internal sample rate to reduce aliasing from non-linear processing (saturation, fast compression).

| Factor | Alias Reduction | CPU Impact | Added Latency |
|--------|----------------|------------|---------------|
| 1x (off) | None | Baseline | 0 |
| 2x | Good | ~+50% | ~8 samples |
| 4x | Better | ~+150% | ~16 samples |
| 8x | Best | ~+350% | ~32 samples |

### Eco Mode

Reduces CPU usage by disabling oversampling and lowering internal update rates. Useful when running many instances.

### Auto-Release

Enables program-dependent release that automatically adjusts based on the input signal. Fast release for transients, slow release for sustained material.

### Auto-Makeup

Automatically calculates and applies makeup gain based on threshold and ratio settings to maintain perceived loudness.

### Dual-Stack

Enables two serial compression stages:
1. **Stage 1 (FET-style):** Fast attack/release for transient control
2. **Stage 2 (Opto-style):** Slow attack/release for smooth leveling

### M/S Mode

Switches from Left/Right to Mid/Side processing. Compresses the mid (center) and side (stereo width) signals independently.

---

## Metering Guide

### Input Meter

Displays the peak input level in dBFS. The meter shows the signal level before any compression is applied.

- **Green zone (below -12 dB):** Healthy headroom
- **Yellow zone (-12 to -6 dB):** Moderate level
- **Red zone (above -6 dB):** Hot signal, watch for clipping

### Output Meter

Displays the peak output level after compression, makeup gain, and mix. Match this to your desired output level.

### Gain Reduction Meter

Shows the amount of compression being applied in real time, displayed as negative dB values. The meter moves downward as compression increases.

- **1-3 dB GR:** Gentle, transparent compression
- **3-6 dB GR:** Moderate, audible compression
- **6-12 dB GR:** Heavy compression
- **12+ dB GR:** Extreme, limiting territory

### Compression Curve Display

Visual representation of the input/output transfer function. Shows the threshold point, knee shape, and ratio slope. The current signal position is indicated on the curve.

---

## Preset System

### Browsing Presets

Use the preset browser in the top bar to navigate factory and user presets. Presets are organized by category:

- **Vocal** — Pop Vocal Shine, Retro Croon Warmth, Vocal Leveler, Podcast Voice
- **Drum** — FET Fast Punch, Drum Punch Stack, Snare Snap
- **Bass** — Bass Control
- **Guitar** — Acoustic Gentle, Guitar Sustain
- **Keys** — Piano Sustain
- **Mix Bus** — VCA Bus Glue
- **Master** — Master Glue
- **Creative** — Bedroom Bloom, Parallel Crush, EDM Sidechain, Vintage Warmth, Lo-Fi Squeeze
- **Utility** — Clean Transparent, De-Esser Trick
- **General** — Default, Opto Smooth

### Loading Presets

Click a preset name to load it instantly. All parameters update to the preset values. Your previous settings are stored in the undo history.

### Saving Presets

1. Dial in your desired settings.
2. Click the **Save** button in the preset browser.
3. Enter a name, select a category, and optionally add tags and a description.
4. Click **Save**. The preset appears in your user presets folder.

### User Preset Location

| Platform | Path |
|----------|------|
| macOS | `~/Library/Application Support/EzSqueeze/Presets/` |
| Windows | `%APPDATA%\EzSqueeze\Presets\` |

### Preset File Format

Presets are stored as JSON files following the schema defined in `assets/presets/schema.json`. You can edit preset files directly with a text editor.

---

## Advanced Features

### Dual-Stage Compression

When Dual-Stack is enabled, the signal passes through two serial compression stages:

**Stage 1 — FET Character (Fast)**
- Attack: 0.1 – 5 ms
- Release: 50 – 150 ms
- Catches transients and peak events

**Stage 2 — Opto Character (Slow)**
- Attack: 5 – 30 ms
- Release: 200 – 1000 ms
- Smooths overall dynamics for even leveling

The main threshold and ratio controls affect both stages proportionally. This approach is ideal for vocals (consistent level + controlled peaks) and drums (punch + body).

### Sidechain Filtering

The SC HPF and SC LPF controls filter the detection signal without affecting the audio path. This lets you control what frequencies drive the compression:

- **HPF at 60-100 Hz:** Prevents kick drum and bass from pumping the compressor on bus/master
- **HPF at 300 Hz + LPF at 8000 Hz:** Creates a de-esser effect (see De-Esser Trick preset)
- **LPF at 2000 Hz:** Makes the compressor respond only to low/mid content

### Mid/Side Processing

M/S mode processes the center (mid) and stereo width (side) independently:

- **Mid compression:** Controls the center image (vocals, kick, bass, snare)
- **Side compression:** Controls the stereo width (reverbs, panned elements, room mics)

Use cases:
- Compress the mid harder to tighten the center without affecting width
- Compress the side gently to control reverb tails
- Different ratios for mid vs. side for creative stereo shaping

### Oversampling

Enable oversampling when using high vibe/saturation settings or ultra-fast attack times. This reduces aliasing artifacts at the cost of CPU. For tracking/mixing, 2x is usually sufficient. For mastering, try 4x or 8x.

### Transient Sculpting

The Snap, Body, and De-Snap controls work together to shape the compressor's transient response:

- **High Snap + Low Body:** Emphasizes the attack, punchy drums
- **Low Snap + High Body:** Emphasizes sustain, smooth sustaining instruments
- **High De-Snap:** Softens harsh transients (pick noise, consonants)

---

## Tips & Tricks

### Vocal Chain

1. Start with the **Podcast Voice** preset for spoken word, or **Pop Vocal Shine** for singing.
2. Set threshold for 3-5 dB of gain reduction on the loudest phrases.
3. Enable Auto-Makeup for consistent output level.
4. Add 60-80 Hz on SC HPF to ignore plosives.
5. If sibilance is a problem, try the **De-Esser Trick** preset on a second instance.

### Drum Bus

1. Start with **VCA Bus Glue** or **FET Fast Punch**.
2. Use attack 10-30 ms to let the transients punch through.
3. Set release to match the tempo: faster for uptempo, slower for ballads.
4. Try Parallel Crush at 30-40% mix for added weight without losing dynamics.

### Mastering

1. Use **Master Glue** as a starting point.
2. Keep gain reduction under 2-3 dB for transparent glue.
3. Enable 2x or 4x oversampling for the highest quality.
4. Set SC HPF to 60 Hz to prevent bass from pumping the master.
5. Use Soft knee for the most transparent compression curve.

### Parallel Compression (NY-Style)

1. Load **Parallel Crush** or set up manually: low threshold, high ratio, fast attack/release.
2. Reduce Mix to 30-50%.
3. The dry signal maintains dynamics while the compressed signal adds density and sustain.

### Getting More Character

1. Increase Vibe Wheel for harmonic saturation.
2. Switch Impedance to Tube for warm even harmonics or Transformer for a broader spectrum.
3. Add Cloud Gain to push the signal harder into the character stage.
4. Enable oversampling to keep aliasing under control at high saturation levels.

### CPU Optimization

1. Enable Eco Mode on instances where quality isn't critical.
2. Reduce oversampling factor (use 1x for tracking, increase for final mix).
3. Increase your DAW's buffer size for mixing sessions.
4. Freeze or bounce tracks with heavy processing.

---

## Troubleshooting

### High CPU Usage

- Disable oversampling or reduce to 2x.
- Enable Eco Mode.
- Increase DAW buffer size.
- Freeze tracks with EzSqueeze if not actively tweaking.

### Latency Issues

EzSqueeze introduces latency when Lookahead or Oversampling is enabled. Most DAWs compensate automatically via Plugin Delay Compensation (PDC).

| Feature | Approximate Latency |
|---------|-------------------|
| Lookahead 5 ms @ 48 kHz | 240 samples (5 ms) |
| 2x Oversampling | ~8 samples (< 1 ms) |
| 4x Oversampling | ~16 samples (< 1 ms) |
| 8x Oversampling | ~32 samples (< 1 ms) |

If you hear timing issues during live monitoring, disable Lookahead and Oversampling.

### Plugin Not Appearing in DAW

1. Verify the plugin files are in the correct directory (see Installation).
2. Rescan plugins in your DAW's plugin manager.
3. Check the DAW's plugin scan log for error messages.
4. On macOS, check System Settings > Privacy & Security if the plugin is blocked.
5. Ensure you're using the 64-bit version of your DAW.

### Audio Crackling or Glitches

- Increase DAW buffer size (512 or 1024 samples).
- Reduce oversampling factor.
- Enable Eco Mode.
- Close other CPU-intensive applications.
- Check that your audio interface drivers are up to date.

### DAW Compatibility

EzSqueeze has been tested with:
- **macOS:** Logic Pro, Ableton Live, Pro Tools, Reaper, Studio One, Cubase, FL Studio
- **Windows:** Ableton Live, Pro Tools, Reaper, Studio One, Cubase, FL Studio, Bitwig

---

## Keyboard Shortcuts

| Action | Shortcut |
|--------|----------|
| Bypass plugin | `B` |
| Reset parameter to default | Double-click knob |
| Fine-tune parameter | Ctrl/Cmd + drag knob |
| Enter exact value | Double-click parameter value |
| Undo | Ctrl/Cmd + Z |
| Redo | Ctrl/Cmd + Shift + Z |
| A/B compare | `A` |
| Copy current state to B | Ctrl/Cmd + Shift + A |
| Previous preset | Up Arrow (in preset browser) |
| Next preset | Down Arrow (in preset browser) |

---

*EzSqueeze is developed by Isaac Hernandez. For support, visit the project repository or file an issue on GitHub.*

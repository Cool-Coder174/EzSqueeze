# 🍊 EzSqueeze — Citrus Dynamics Compressor

**Professional dynamics processing for indie musicians and small producers.**

[![Build Status](https://img.shields.io/badge/build-in--development-yellow)](https://github.com/yourusername/EzSqueeze)
[![License](https://img.shields.io/badge/license-GPL--3.0-orange)](LICENSE)
[![Version](https://img.shields.io/badge/version-1.0.0--beta.1-brightgreen)](CHANGELOG.md)

---

## Features

### 🎛️ Core Compression
- **Detector modes**: Peak & RMS with program-dependent release
- **Flexible knee**: Hard, medium, soft transitions
- **Lookahead**: 0–10ms with automatic latency compensation
- **Stereo control**: Link % and full M/S processing mode
- **Sidechain**: External input with HPF/LPF filtering

### 🔥 Character & Color
- **Dual-stage path**: FET-fast + Opto-slow serial compression
- **Cloud-Gain preamp**: +0 to +30dB with impedance character (Silicon/Tube/Transformer)
- **Transient sculptor**: Snap, Body, De-Snap controls
- **Vibe Wheel**: Seamless blend from clean to vintage warmth

### 🎨 Modern UX
- **Citrus liquid-glass theme**: Inspired by Apple's design language with vibrant citrus accents
- **Comprehensive metering**: Input/output/GR with peak hold & history trail
- **Preset browser**: Tagged, searchable factory + user presets
- **A/B comparison**: Quick snapshot system with undo/redo
- **Tooltips & accessibility**: Full keyboard navigation, scalable 90–130%

### ⚙️ Technical Excellence
- **Oversampling**: 2×/4×/8× with polyphase filters
- **CPU-efficient**: Eco Mode for low-power systems
- **Sample-accurate**: Automation with click-free bypass
- **Cross-platform**: macOS (Intel/ARM Universal) & Windows VST3/AU

---

## Project Status

**Current Phase**: All Phases Complete ✅

All 8 phases of the roadmap have been implemented. The project includes a complete C++17 DSP engine (14 classes), professional HISE UI (900x650, citrus liquid-glass theme), 22 factory presets, CMake/JUCE build system, GitHub Actions CI/CD, Catch2 unit tests, and comprehensive documentation.

See [report.md](report.md) for detailed project status and roadmap.

---

## Installation

> **Note**: Binary releases are not yet available. The plugin is currently in development.

### macOS (Future)
1. Download `EzSqueeze-v1.0.0-macOS-Universal.dmg`
2. Open and drag to `/Library/Audio/Plug-Ins/VST3/` (VST3) or `Components/` (AU)
3. Rescan plugins in your DAW

### Windows (Future)
1. Download `EzSqueeze-v1.0.0-Windows-x64.zip`
2. Extract to `C:\Program Files\Common Files\VST3\`
3. Rescan plugins in your DAW

---

## Building from Source

### Prerequisites
- **CMake 3.15+** — [cmake.org](https://cmake.org/download/) or `winget install cmake`
- **C++17 compiler:** Xcode 12+ (macOS) or Visual Studio 2019+ (Windows, with “Desktop development with C++”)
- **Ninja (recommended on Windows):** `choco install ninja` — not required if using Visual Studio generator

**JUCE is fetched automatically** by CMake (no manual install). See [docs/DEPENDENCIES.md](docs/DEPENDENCIES.md) for full details.

### Quick build (Windows PowerShell)

From the project root (where `build.ps1` and `CMakeLists.txt` live):

```powershell
.\build.ps1           # Configure and build Release
.\build.ps1 -Clean    # Clean build directory, then configure and build
.\build.ps1 -Tests    # Build and run unit tests
```

Output: `build/EzSqueeze_artefacts/Release/` (VST3, and AU on macOS).

### Manual CMake build

```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel
```

More options (e.g. Visual Studio, tests): [docs/BUILD.md](docs/BUILD.md).

### HISE (optional, for UI prototyping)

- **HISE** is optional — [Download](https://hise.audio)
- Open `hise/xml/EZSqueezeV2.xml` in HISE for interface scripting; the **JUCE build above** produces the distributable VST3/AU.

---

## Usage

### Quick Start (When Available)
1. **Insert** EzSqueeze on your vocal, drum bus, or master channel
2. **Select a style**: Choose from "Pop Vocal Shine", "Drum Glue", etc.
3. **Adjust threshold**: Aim for 2–4dB gain reduction (watch the GR meter)
4. **Tweak attack/release**: Fast for transients, slow for smooth leveling
5. **Use Vibe Wheel**: Dial in clean or vintage character to taste

### Tips
- **Parallel compression**: Lower the Mix knob to ~50% for gentle glue
- **Sidechain filtering**: Use SC HPF (80–120Hz) to avoid bass pumping
- **Auto Sweet Spot**: Let the plugin analyze and set optimal threshold
- **Dual Stack**: Enable for vocal "leveling + polish" in one plugin

---

## Documentation

- [Project Report](report.md) — Current status, roadmap, and implementation plan
- [Architecture](docs/ARCHITECTURE.md) — Codebase layout, signal flow, and module overview
- [Build Guide](docs/BUILD.md) — Prerequisites, build.ps1 usage, and troubleshooting
- [Technical Notes](docs/TECH_NOTES.md) — DSP algorithms and math
- [UI Design Guide](docs/DESIGN_UI.md) — Layout, theme, accessibility
- [Contributing](CONTRIBUTING.md) — Code standards & PR process
- [Changelog](CHANGELOG.md) — Version history
- [Dependencies](docs/DEPENDENCIES.md) — Build requirements
- [Audit log](docs/AUDIT.md) — Codebase audit and fixes (March 2026)

---

## Development Roadmap

- [x] **Phase 0**: Foundation & Repository Overhaul (Week 1)
- [x] **Phase 1**: DSP Core Implementation (Weeks 2-4)
- [x] **Phase 2**: Advanced Features (Weeks 5-6)
- [x] **Phase 3**: UX Overhaul (Weeks 7-8)
- [x] **Phase 4**: Presets & Modes (Week 9)
- [x] **Phase 5**: Testing (Weeks 10-11)
- [x] **Phase 6**: Build System & CI/CD (Week 12)
- [x] **Phase 7**: Documentation (Week 13)
- [x] **Phase 8**: Final Polish & Release (Week 14)

---

## Contributing

We welcome contributions! Please read:
- [CONTRIBUTING.md](CONTRIBUTING.md) for development guidelines
- [CODE_OF_CONDUCT.md](CODE_OF_CONDUCT.md) for community standards

### Quick Contribution Guide
1. Fork the repository
2. Create a feature branch: `git checkout -b feature/my-feature`
3. Follow our coding standards (see `.clang-format`)
4. Add tests for new features
5. Submit a pull request

---

## Support

- **Issues**: [GitHub Issues](https://github.com/yourusername/EzSqueeze/issues)
- **Discussions**: [GitHub Discussions](https://github.com/yourusername/EzSqueeze/discussions)
- **Email**: support@yourdomain.com

---

## License

GPL-3.0 — See [LICENSE](LICENSE) for details.

---

## Credits

**Developed with ❤️ by Isaac Hernandez**

**Design**: Citrus Liquid Glass theme inspired by Apple's design language  
**Special Thanks**: HISE community, JUCE framework, and all contributors

---

## Color Palette

Our signature citrus palette:
- Primary: `#EF932C` 🍊
- Light: `#F1B233` 🌟
- Dark: `#DA761E` 🧡
- Accent: `#EE811C` ✨

---

**EzSqueeze** — Professional dynamics, citrus style. 🍊


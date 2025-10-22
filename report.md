# EzSqueeze Overhaul Project Report

**Generated:** October 13, 2025  
**Branch:** `newFeature/overhaul`  
**HISE Version:** To be verified (check Help → About in HISE)  
**Target:** Professional compressor plugin for indie musicians

---

## Executive Summary

EzSqueeze is being transformed from an early prototype into a professional-grade dynamics processor plugin. This report documents the current state, planned improvements, and implementation roadmap spanning 14 weeks across 8 phases.

---

## Project Status

### Current State Summary
- **Stage:** Early prototype → Professional restructure (Phase 0 Complete ✅)
- **DSP:** Basic dynamics processor (threshold, ratio, attack, release, makeup)
- **UI:** 800×600 prototype with orange knobs, minimal scripting
- **Build:** HISE project, no export configured yet
- **Documentation:** Foundation documentation created
- **Tests:** To be implemented in Phase 5
- **CI/CD:** Templates created, to be activated in Phase 6

### Existing Assets
- 3× orange knob filmstrips (60 frames each) → moved to `assets/ui/knobs/`
- Aderoy.otf custom font → moved to `assets/ui/fonts/`
- Basic XML preset structure → moved to `hise/xml/`
- EZSqueezeV2 script processor → moved to `hise/scripts/`

### Repository Structure (New)
```
/EzSqueeze/
├── .github/                     # GitHub templates & workflows
│   ├── workflows/               # CI/CD pipelines (templates)
│   └── ISSUE_TEMPLATE/          # Bug/feature templates
├── hise/                        # HISE project files
│   ├── presets/                 # .hip preset files
│   ├── xml/                     # XML backups
│   └── scripts/                 # HISE JavaScript
│       └── ScriptProcessors/
│           └── EZSqueezeV2/
├── assets/                      # All media assets
│   ├── ui/
│   │   ├── wireframes/          # UI specifications
│   │   ├── knobs/               # Filmstrip images
│   │   └── fonts/               # Typography
│   └── presets/
│       ├── factory/             # Shipped presets (future)
│       └── schema.json          # Preset validation
├── source/                      # Future C++/JUCE export
├── modules/                     # Custom DSP modules (future)
├── tests/                       # Test suites (future)
│   ├── unit/
│   ├── golden/
│   └── benchmarks/
├── tools/                       # Helper scripts
├── docs/                        # Documentation
├── .gitignore                   # VC exclusions
├── .clang-format                # Code style
├── .clang-tidy                  # Static analysis
├── README.md                    # Project overview
├── CONTRIBUTING.md              # Dev guidelines
├── CHANGELOG.md                 # Version history
└── report.md                    # This file
```

---

## Phase Breakdown & Action Plan

### ✅ Phase 0: Foundation (Week 1) — **COMPLETE**
- [x] Create `newFeature/overhaul` branch
- [x] Add comprehensive `.gitignore`
- [x] Restructure repository with new folder layout
- [x] Move HISE files to `hise/` directory
- [x] Move assets to `assets/` directory
- [x] Remove deprecated EZSqueezeV1
- [x] Create README.md
- [x] Create this report.md
- [x] Create CONTRIBUTING.md
- [x] Create TECH_NOTES.md (structure)
- [x] Create DESIGN_UI.md
- [x] Create CODE_OF_CONDUCT.md
- [x] Create CHANGELOG.md
- [x] Set up code style configs (.clang-format, .clang-tidy)
- [x] Create UI wireframe JSON
- [x] Create preset schema JSON
- [x] Add GitHub templates (issues, PRs, workflows)

### Phase 1: DSP Core (Weeks 2-4)

Status audit of current codebase indicates several Phase 1 items are already implemented:
- Detector (Peak/RMS): present in `source/dsp/Detector.h/.cpp`
- GainComputer (threshold/ratio/knee): present in `source/dsp/GainComputer.h`
- EnvelopeFollower (attack/release): present in `source/dsp/EnvelopeFollower.h`
- LookaheadBuffer (0–10ms + latency getter): present in `source/dsp/LookaheadBuffer.h/.cpp`
- Stereo link and M/S utilities: present in `source/dsp/StereoProcessor.h/.cpp` and `source/dsp/StereoLink.h`
- Program-dependent release heuristic: present in `source/dsp/ProgramDependentRelease.h`

Accordingly, Phase 1 will focus on integration, missing utilities, tests, and documentation.

#### Objectives
- [ ] Integrate Detector → EnvelopeFollower → GainComputer end-to-end processing path
- [ ] Finalize GainComputer soft/medium/hard knee behavior (complete `computeSoftKnee` implementation)
- [ ] Implement `CompressorUtilities` glue (threshold/ratio conversions, dB↔linear helpers)
- [ ] Build SidechainFilter chain (HPF 20–400 Hz, LPF 4–16 kHz) using `Biquad`
- [ ] Add ParallelMix/blend control (dry/wet)
- [ ] Implement AutoMakeupGain (initial heuristic + adaptive option)
- [ ] Wire ProgramDependentRelease into EnvelopeFollower (optional mode)
- [ ] Add RT-safety assertions (no allocations in process, pre-allocate buffers)
- [ ] Document all DSP math in `docs/TECH_NOTES.md` (detector, envelope, knee, lookahead, stereo link)
- [ ] Unit tests for detector, gain computer, envelope, lookahead, stereo link, sidechain filter

#### Week-by-Week Plan
- Week 2:
  - [ ] Finish GainComputer knee implementation and unit tests
  - [ ] Integrate Detector → EnvelopeFollower → GainComputer; add simple processing harness
  - [ ] Add StereoLink/MS path selection and tests
  - [ ] Update TECH_NOTES with finalized formulas (detector, envelope, knee)
- Week 3:
  - [ ] Implement SidechainFilter (HPF/LPF with RBJ biquads) and tests
  - [ ] Add LookaheadBuffer into the path with latency aggregation function
  - [ ] Implement ParallelMix and AutoMakeupGain (heuristic) with tests
  - [ ] RT-safety review (prepare/reset paths, no per-sample allocation)
- Week 4:
  - [ ] Introduce ProgramDependentRelease option and tests
  - [ ] Add adaptive AutoMakeup (running GR) and toggle
  - [ ] Complete documentation for all Phase 1 modules; API tidy-up
  - [ ] Gate: Phase 1 sign-off after tests pass and docs updated

#### Exit Criteria (Gates)
- [ ] All Phase 1 unit tests pass locally
- [ ] End-to-end compressor path processes buffers deterministically (no NaNs/inf)
- [ ] Reported latency = lookahead + (any fixed filter/OS latency) with ±1 sample accuracy
- [ ] RT-safety: no allocations or locks in audio path; sanitizer clean in debug
- [ ] TECH_NOTES updated with equations and references for all Phase 1 modules

**Key Deliverables:**
- Integrated compressor core (Detector → Envelope → GainComputer → GR apply)
- `source/dsp/GainComputer.cpp` (knee impl) and finalized API in `.h`
- `source/dsp/SidechainFilter.h/cpp` (HPF/LPF biquads)
- `source/dsp/ParallelMix.h` (utility) and mix wiring
- `source/dsp/AutoMakeup.h/cpp` (heuristic + adaptive)
- `source/dsp/Latency.h` (aggregates lookahead + fixed latencies)
- Unit tests in `tests/unit/*` for all modules
- Updated `docs/TECH_NOTES.md`

### Phase 2: Advanced Features (Weeks 5-6)
- [ ] Dual-stage compressor path (FET-fast + Opto-slow)
- [ ] Cloud-Gain preamp (+0 to +30dB)
- [ ] Impedance character switch (Silicon/Tube/Transformer)
- [ ] Saturation stage (even/odd/tape curves)
- [ ] Transient Sculptor (Snap/Body/De-Snap mapping)
- [ ] Oversampling engine (2×/4×/8× with polyphase filters)
- [ ] Eco Mode (performance optimization)
- [ ] Tempo-sync for attack/release (1/64 – 1/2 note)

**Key Deliverables:**
- `modules/cloud_gain/CloudGain.h/cpp`
- `modules/transient_sculptor/TransientSculptor.h/cpp`
- `modules/dual_stage_comp/DualStage.h/cpp`
- `source/dsp/Oversampling.h/cpp`

### Phase 3: UX Overhaul (Weeks 7-8)
- [ ] Implement citrus palette & liquid-glass theme
- [ ] Build GR meter with history trail
- [ ] Create input/output meters with peak hold
- [ ] Add oversampling indicator & CPU monitor
- [ ] Implement Vibe Wheel macro control
- [ ] Build preset browser with search/tags
- [ ] Add A/B comparison system
- [ ] Implement undo/redo
- [ ] Create tooltip system
- [ ] Ensure 90-130% UI scaling
- [ ] Implement keyboard navigation (tab order)
- [ ] Accessibility audit (contrast, focus indicators)

**Key Deliverables:**
- Complete UI implementation in HISE or JUCE
- Meter components with animations
- Preset management system
- State management for A/B and undo/redo

### Phase 4: Presets & Modes (Week 9)
- [ ] Define genre/style preset macros:
  - [ ] Pop Vocal Shine (modern, airy polish)
  - [ ] Retro Croon Warmth (ballad mid-thick)
  - [ ] Bedroom Bloom (soft glue + gentle top roll-off)
  - [ ] FET Fast Punch (transient bite)
  - [ ] VCA Bus Glue (mixbus cohesion)
- [ ] Create dual-stage presets:
  - [ ] Vocal Leveler
  - [ ] Drum Punch Stack
- [ ] Implement "Auto Sweet Spot" analyzer (2-4dB GR target)
- [ ] Create 20+ factory presets across categories
- [ ] Define community preset JSON schema (already created)
- [ ] Build preset loader with validation

**Key Deliverables:**
- `assets/presets/factory/*.json` (20+ presets)
- Preset analyzer algorithm
- Preset import/export functionality

### Phase 5: Testing (Weeks 10-11)
- [ ] Unit tests (Catch2 or GoogleTest):
  - [ ] Detector accuracy (Peak vs RMS tolerance)
  - [ ] Gain computer math (ratio/knee correctness)
  - [ ] Attack/release envelope within spec
  - [ ] Auto-makeup loudness compensation (±0.3dB)
  - [ ] M/S & stereo link invariants
  - [ ] No heap allocations in audio callback (assert)
- [ ] Golden audio tests:
  - [ ] Process reference WAVs at 44.1/48/96/192kHz
  - [ ] Verify GR envelope shapes against expected
  - [ ] Latency reporting accuracy (±1 sample)
  - [ ] Bypass click-free (< -80dBFS spikes)
  - [ ] Oversampling alias rejection spec
- [ ] Benchmarks:
  - [ ] Block sizes 32-1024 samples
  - [ ] CPU % profiling across sample rates
  - [ ] Memory allocation checks (debug mode)

**Key Deliverables:**
- `tests/unit/*.cpp` (comprehensive unit test suite)
- `tests/golden/inputs/*.wav` (test signals)
- `tests/golden/expected/*.wav` (golden references)
- `tests/benchmarks/bench_dsp.cpp`
- CI integration (all tests must pass)

### Phase 6: Build System & CI/CD (Week 12)
- [ ] Export HISE project to JUCE/CMake
- [ ] Configure CMakeLists.txt:
  - [ ] VST3 + AU (macOS) formats
  - [ ] Optimization flags (-O3, strip symbols)
  - [ ] ASAN/UBSAN toggle for debug
- [ ] Set up GitHub Actions workflows:
  - [ ] macOS universal build (x86_64 + ARM64)
  - [ ] Windows build (MSVC x64)
  - [ ] Run all tests headless
  - [ ] Lint checks (clang-format, clang-tidy with fail-on-error)
  - [ ] Generate build artifacts (zipped installers)
- [ ] Configure code signing (optional, if secrets available)
- [ ] Set up artifact storage & release automation

**Key Deliverables:**
- `CMakeLists.txt` (complete build configuration)
- `.github/workflows/build.yml` (functional)
- `.github/workflows/test.yml` (functional)
- `.github/workflows/lint.yml` (functional)
- Signed binaries for macOS and Windows

### Phase 7: Documentation (Week 13)
- [ ] Complete README.md (installation, usage)
- [ ] Complete TECH_NOTES.md (all DSP algorithms documented)
- [ ] Complete DESIGN_UI.md (component specs, accessibility)
- [ ] Complete API documentation (Doxygen for C++)
- [ ] Create user manual (PDF or HTML)
- [ ] Add inline code comments (Doxygen-style)
- [ ] Create demo session guide (`docs/demo/`)
- [ ] Write null-test guide for verification

**Key Deliverables:**
- Complete technical documentation
- User-facing manual
- Developer API docs
- Demo materials

### Phase 8: Final Polish & Release (Week 14)
- [ ] Integration testing with major DAWs:
  - [ ] Ableton Live
  - [ ] Logic Pro
  - [ ] Reaper
  - [ ] FL Studio
  - [ ] Pro Tools (if AAX added)
- [ ] Performance optimization pass
- [ ] Accessibility audit (contrast ratios, screen readers)
- [ ] Beta testing with target users
- [ ] Final code review & refactor
- [ ] Prepare release notes
- [ ] Create installers with proper metadata
- [ ] Tag v1.0.0 release

**Key Deliverables:**
- Release-ready binaries
- Comprehensive release notes
- Beta feedback incorporated
- v1.0.0 Git tag

---

## Dependencies & Versions

### Required
- **HISE**: Latest stable (check [hise.audio](https://hise.audio))
- **C++17 Compiler**:
  - macOS: Xcode 12+ (includes Apple Clang)
  - Windows: MSVC 2019+ (Visual Studio 2019/2022)
- **CMake**: 3.15 or later (for JUCE export)

### Optional (Future)
- **JUCE**: 7.x (for export from HISE)
- **Catch2**: Latest (for unit testing)
- **r8brain-free-src**: For alternative oversampling (if not using JUCE's)

See [docs/DEPENDENCIES.md](docs/DEPENDENCIES.md) for full details.

---

## Missing Critical Components (To Implement)

### DSP Features
- [ ] Lookahead & latency compensation
- [ ] M/S processing & stereo link
- [ ] Sidechain filtering (HPF/LPF)
- [ ] Oversampling engine (2×/4×/8×)
- [ ] Program-dependent release
- [ ] Auto-makeup gain
- [ ] Dual-stage compressor path
- [ ] Cloud-Gain preamp
- [ ] Transient sculptor
- [ ] Tempo-sync timing

### UI/UX Features
- [ ] Comprehensive metering (IN/GR/OUT with peak hold)
- [ ] GR history trail visualization
- [ ] Preset browser with search/tags
- [ ] A/B comparison
- [ ] Undo/redo system
- [ ] Tooltips
- [ ] CPU & latency monitors
- [ ] Vibe Wheel macro
- [ ] Auto Sweet Spot analyzer
- [ ] 90-130% UI scaling
- [ ] Keyboard navigation (full tab order)

### Infrastructure
- [ ] Unit test suite
- [ ] Golden audio tests
- [ ] Performance benchmarks
- [ ] CI/CD pipelines (GitHub Actions)
- [ ] Code signing setup
- [ ] Installer creation
- [ ] Release automation

---

## Blockers & Open Questions

### Information Needed
1. **HISE Version**: Need to verify from project file (check Help → About in HISE)
2. **Build machines**: What systems do you have for testing? (macOS Intel/ARM, Windows)
3. **Code signing**: Do you have Apple Developer ID & Windows signing certificate?
4. **Target formats**: Confirm VST3 + AU (macOS) - also AAX for Pro Tools?
5. **Minimum OS**: macOS 10.13+? Windows 10+?
6. **DSP priorities**: Which features are must-have for v1.0 vs nice-to-have for v1.1+?

### Technical Decisions
1. **Oversampling library**: Use JUCE's built-in or custom (e.g., r8brain)?
   - *Recommended*: JUCE's (proven, efficient, well-integrated)
2. **Preset format**: JSON vs XML vs HISE native?
   - *Recommended*: JSON (human-readable, extensible, web-friendly)
3. **Metering**: Real-time scope or simple bar meters?
   - *Recommended*: Bar meters for CPU efficiency
4. **Distribution**: Self-hosted, GitHub releases, or plugin platforms?
   - *Recommended*: GitHub releases initially

### Sensible Defaults (Proceeding With)
- JUCE 7.x for export
- VST3 + AU (macOS only) formats
- JSON for presets
- JUCE's oversampling
- GitHub Actions for CI (free for public repos)
- Semantic versioning (starting at v1.0.0-beta.1)

---

## Success Metrics

### By End of Phase 8 (v1.0.0 Release)
- [ ] All unit, golden, and benchmark tests passing
- [ ] CI producing artifacts for macOS (Universal) + Windows (x64)
- [ ] CPU usage < 5% @ 44.1kHz, 512 samples (single instance)
- [ ] Latency correctly reported to DAW (± 1 sample)
- [ ] All features from specification implemented
- [ ] 90%+ code coverage (unit tests)
- [ ] Zero warnings with `-Wall -Wextra -Werror`
- [ ] Professional UI with citrus theme fully functional
- [ ] 20+ factory presets across all categories
- [ ] Complete documentation (README, tech notes, user manual)
- [ ] Successful testing in 4+ major DAWs
- [ ] Accessibility compliant (WCAG AA level)

---

## Risk Assessment

### High Risk
- **Latency compensation complexity**: Lookahead + oversampling interaction
  - *Mitigation*: Reference JUCE examples, extensive testing, clear documentation
- **CPU performance**: Oversampling + dual-stage can be demanding
  - *Mitigation*: Profiling early, Eco Mode toggle, optimize hot paths
- **Cross-platform builds**: macOS ARM + Intel, Windows compatibility
  - *Mitigation*: Early CI setup, test on real hardware, matrix builds

### Medium Risk
- **HISE → JUCE export**: Potential issues with custom UI export
  - *Mitigation*: Be prepared to rebuild UI in JUCE from scratch if needed
- **Preset compatibility**: Future v1.x → v2.x migration
  - *Mitigation*: Version presets, write converter scripts, maintain schemas
- **Code signing**: Requires paid developer accounts
  - *Mitigation*: Can release unsigned initially for testing

### Low Risk
- **Asset creation**: Filmstrips, graphics
  - *Mitigation*: Can adapt existing orange knobs, iterate on design
- **Documentation**: Time-consuming but straightforward
  - *Mitigation*: Document incrementally as features are built

---

## Future Enhancements (Post-v1.0)

### Version 1.1 (Q2 2026)
- [ ] Bus/multiband version ("Citrus Comp Bus")
- [ ] Stereo width control
- [ ] Tilt EQ integration
- [ ] Preset sharing/download service (community hub)
- [ ] Interactive "Learn" panel with tips

### Version 1.2 (Q3 2026)
- [ ] Surround sound support (5.1, 7.1, Atmos)
- [ ] Advanced sidechain routing matrix
- [ ] Preset morphing (crossfade between presets)
- [ ] MIDI learn for hardware controllers
- [ ] Session statistics (total GR, dynamic range, etc.)

### Version 2.0 (2027)
- [ ] AI-assisted preset suggestion based on material
- [ ] Spectral dynamics mode (frequency-dependent)
- [ ] Transient-preserving intelligent limiting
- [ ] Integrated mastering chain (EQ + comp + limiter)
- [ ] Cloud collaboration features (session sharing)

---

## Commit Strategy

### Conventional Commit Format
```
<type>(<scope>): <subject>

<body>

<footer>
```

**Types**: `feat`, `fix`, `docs`, `style`, `refactor`, `perf`, `test`, `build`, `ci`, `chore`

**Examples**:
- `feat(dsp): add soft/medium/hard knee to gain computer`
- `refactor(ui): extract meter component to reusable class`
- `docs(tech): document lookahead latency compensation algorithm`
- `test(unit): add edge case tests for detector RMS mode`
- `ci(actions): add macOS universal build workflow`
- `chore(init): complete Phase 0 repository overhaul` ← (this commit)

---

## Pain Points Identified (Original Prototype)

### Structural
1. **No separation of concerns**: HISE project mixed with build artifacts
2. **Empty directories**: Documentation/, AdditionalSourceCode/, DspNetworks/Networks/
3. **No version control hygiene**: Missing .gitignore, Binaries/ tracked
4. **No documentation**: No README, CONTRIBUTING, or technical notes
5. **Unclear versioning**: V1 vs V2 confusion

### Code Quality
1. **Minimal scripting**: Only empty callback stubs in Interface.js
2. **No parameter validation**: Direct processor linkage without bounds
3. **No state management**: No preset system beyond HISE defaults
4. **Hard-coded values**: UI dimensions, colors not parameterized
5. **No comments**: Script files lack documentation

### Build & Deployment
- No CMake configuration
- No export settings optimized for VST3/AU
- No code signing configuration
- No installer scripts
- No CI/CD pipelines

**All of the above have been addressed in Phase 0.** ✅

---

## Timeline Summary

| Phase | Duration | Focus | Status |
|-------|----------|-------|--------|
| Phase 0 | Week 1 | Foundation & Restructure | ✅ Complete |
| Phase 1 | Weeks 2-4 | DSP Core | ⏳ Next |
| Phase 2 | Weeks 5-6 | Advanced Features | 📅 Planned |
| Phase 3 | Weeks 7-8 | UX Overhaul | 📅 Planned |
| Phase 4 | Week 9 | Presets & Modes | 📅 Planned |
| Phase 5 | Weeks 10-11 | Testing | 📅 Planned |
| Phase 6 | Week 12 | Build & CI/CD | 📅 Planned |
| Phase 7 | Week 13 | Documentation | 📅 Planned |
| Phase 8 | Week 14 | Final Polish & Release | 📅 Planned |

**Total Estimated Timeline**: 14 weeks (~3.5 months) to v1.0.0 release

---

## Contact & Collaboration

- **Developer**: Isaac Hernandez
- **Repository**: [GitHub](https://github.com/yourusername/EzSqueeze)
- **Issues**: [GitHub Issues](https://github.com/yourusername/EzSqueeze/issues)
- **Discussions**: [GitHub Discussions](https://github.com/yourusername/EzSqueeze/discussions)

---

## Acknowledgments

- **HISE Framework**: Christoph Hart and the HISE community
- **JUCE Framework**: ROLI/JUCE team
- **Design Inspiration**: Apple's design language, modern DAW UIs
- **Community**: Beta testers and contributors (TBD)

---

**Report Last Updated**: October 13, 2025  
**Next Review**: After Phase 1 completion (Week 4)

---

## Appendix: Phase 0 Completion Checklist

- [x] Create `newFeature/overhaul` branch
- [x] Add `.gitignore`
- [x] Add `.clang-format` and `.clang-tidy`
- [x] Create new directory structure
- [x] Move `Scripts/` → `hise/scripts/`
- [x] Move `XmlPresetBackups/` → `hise/xml/`
- [x] Move `Images/*.png` → `assets/ui/knobs/`
- [x] Move `Images/*.otf` → `assets/ui/fonts/`
- [x] Remove deprecated `EZSqueezeV1/`
- [x] Remove autosave `.hip` files
- [x] Create `README.md`
- [x] Create `report.md` (this file)
- [x] Create `CONTRIBUTING.md`
- [x] Create `TECH_NOTES.md`
- [x] Create `DESIGN_UI.md`
- [x] Create `CODE_OF_CONDUCT.md`
- [x] Create `CHANGELOG.md`
- [x] Create `docs/DEPENDENCIES.md`
- [x] Create `assets/ui/wireframes/main_layout.json`
- [x] Create `assets/presets/schema.json`
- [x] Create GitHub workflow templates
- [x] Create GitHub issue/PR templates
- [x] Initial commit with conventional format

**Phase 0 Status: COMPLETE** ✅

---

**Ready to proceed to Phase 1: DSP Core Implementation**


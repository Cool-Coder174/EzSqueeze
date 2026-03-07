# EzSqueeze Build Dependencies

**Version:** 1.0.0  
**Last Updated:** March 2026

---

## Overview

EzSqueeze uses CMake as its build system and fetches most dependencies automatically. This document lists all required and optional dependencies, their versions, and platform-specific requirements.

---

## Required Dependencies

### CMake 3.15+

The build system requires CMake 3.15 or later for `FetchContent` support.

| Platform | Install |
|----------|---------|
| macOS | `brew install cmake` |
| Windows | Download from [cmake.org](https://cmake.org/download/) or `winget install cmake` |
| Linux | `sudo apt install cmake` or `sudo dnf install cmake` |

Verify: `cmake --version`

### C++17 Compiler

EzSqueeze requires full C++17 support including structured bindings, `std::optional`, `std::variant`, and `if constexpr`.

| Platform | Compiler | Minimum Version |
|----------|----------|-----------------|
| macOS | Apple Clang (via Xcode) | Xcode 12+ (Clang 12+) |
| Windows | MSVC | Visual Studio 2019 (v16.8+) |
| Windows | Clang-cl | Clang 10+ |
| Linux | GCC | 9+ |
| Linux | Clang | 10+ |

### JUCE 7.x

JUCE is the primary audio plugin framework. It is fetched automatically via CMake `FetchContent` — no manual installation is needed.

- **Repository:** https://github.com/juce-framework/JUCE
- **Branch/Tag:** `7.0.9` (or latest 7.x)
- **License:** Dual-licensed (GPL v3 / Commercial)
- **Modules used:** `juce_audio_basics`, `juce_audio_processors`, `juce_audio_formats`, `juce_audio_utils`, `juce_dsp`, `juce_gui_basics`, `juce_gui_extra`

If you prefer a local JUCE installation, set `JUCE_DIR` in your CMake configuration:

```bash
cmake -B build -DJUCE_DIR=/path/to/JUCE
```

---

## Platform-Specific Requirements

### macOS

- **Xcode 12+** with Command Line Tools
- **macOS SDK 10.13+** (High Sierra minimum deployment target)
- **Homebrew** (recommended for tooling)

Install Xcode CLT:
```bash
xcode-select --install
```

Required frameworks (provided by macOS SDK):
- CoreAudio, CoreMIDI, AudioToolbox
- Cocoa, WebKit, IOKit
- Accelerate (for SIMD/vDSP)

### Windows

- **Visual Studio 2019+** (Community edition or higher)
  - Workload: "Desktop development with C++"
  - Individual components: Windows 10/11 SDK, MSVC v142+ build tools
- **Windows SDK 10.0.18362+**

Alternatively, build from command line with:
```cmd
cmake -B build -G "Visual Studio 16 2019" -A x64
cmake --build build --config Release
```

### Linux (optional target)

- **ALSA development libraries:** `sudo apt install libasound2-dev`
- **JACK (optional):** `sudo apt install libjack-jackd2-dev`
- **FreeType:** `sudo apt install libfreetype6-dev`
- **X11/GLX:** `sudo apt install libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libgl1-mesa-dev`
- **Webkit (for JUCE WebView):** `sudo apt install libwebkit2gtk-4.0-dev`
- **cURL:** `sudo apt install libcurl4-openssl-dev`

---

## Test Dependencies

### Catch2 v3

The unit test framework is fetched automatically via CMake `FetchContent` when building tests.

- **Repository:** https://github.com/catchorg/Catch2
- **Tag:** `v3.5.2`
- **License:** BSL-1.0
- **Used for:** Unit tests and performance benchmarks

Build and run tests:
```bash
cmake -B build-tests -S tests
cmake --build build-tests
cd build-tests && ctest --output-on-failure
```

---

## Development Dependencies

### HISE

HISE is used for UI scripting and rapid prototyping during development. It is not required for building the final plugin.

- **Website:** https://hise.audio/
- **Version:** Latest stable
- **Purpose:** Interface scripting, preset design, prototyping
- **License:** GPL v3

### clang-format 14+ (optional)

For consistent code formatting. A `.clang-format` configuration file is provided at the repository root.

```bash
# macOS
brew install clang-format

# Windows (via LLVM)
winget install LLVM.LLVM

# Linux
sudo apt install clang-format
```

Format all source files:
```bash
find source/ -name "*.h" -o -name "*.cpp" | xargs clang-format -i
```

### clang-tidy (optional)

Static analysis tool for catching common issues. A `.clang-tidy` configuration is provided at the repository root.

```bash
# macOS
brew install llvm

# Linux
sudo apt install clang-tidy
```

Run analysis:
```bash
clang-tidy source/dsp/*.h -- -std=c++17 -I/path/to/juce/modules
```

---

## Dependency Summary

| Dependency | Version | Required | Fetched Automatically |
|-----------|---------|----------|----------------------|
| CMake | 3.15+ | Yes | No (install manually) |
| C++17 Compiler | See table above | Yes | No (install manually) |
| JUCE | 7.x | Yes | Yes (FetchContent) |
| Catch2 | v3.5.2 | Tests only | Yes (FetchContent) |
| HISE | Latest | Dev only | No (manual install) |
| clang-format | 14+ | No | No (manual install) |
| clang-tidy | 14+ | No | No (manual install) |

---

## Troubleshooting

### CMake can't find compiler

Ensure your compiler is on `PATH`. On Windows, run CMake from a Developer Command Prompt or use `-G` to specify the generator.

### JUCE fetch fails

If FetchContent times out behind a firewall, clone JUCE manually and point to it:
```bash
git clone https://github.com/juce-framework/JUCE.git /opt/JUCE
cmake -B build -DJUCE_DIR=/opt/JUCE
```

### Catch2 fetch fails

Same approach — clone manually and set `Catch2_DIR`:
```bash
git clone https://github.com/catchorg/Catch2.git /opt/Catch2
cmake -B build-tests -S tests -DCatch2_DIR=/opt/Catch2
```

### Missing Windows SDK

Open Visual Studio Installer and ensure "Windows 10 SDK" or "Windows 11 SDK" is checked under Individual Components.

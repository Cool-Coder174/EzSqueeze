# Contributing to EzSqueeze

Thank you for your interest in contributing to EzSqueeze! We welcome contributions from everyone, whether you're fixing bugs, adding features, improving documentation, or suggesting enhancements.

---

## Code of Conduct

Please read and follow our [CODE_OF_CONDUCT.md](CODE_OF_CONDUCT.md) to ensure a welcoming and inclusive environment for all contributors.

---

## Getting Started

### 1. Fork & Clone

```bash
# Fork the repository on GitHub, then clone your fork
git clone https://github.com/YOUR_USERNAME/EzSqueeze.git
cd EzSqueeze

# Add upstream remote
git remote add upstream https://github.com/ORIGINAL_OWNER/EzSqueeze.git
```

### 2. Create a Branch

```bash
# Always branch from newFeature/overhaul (or main once merged)
git checkout newFeature/overhaul
git pull upstream newFeature/overhaul
git checkout -b feature/my-new-feature
```

### 3. Make Changes

Follow our coding standards (see below) and make your changes with clear, incremental commits.

### 4. Test Thoroughly

```bash
# Run unit tests
cd build
ctest --output-on-failure

# Run linters
clang-format --dry-run --Werror source/**/*.{h,cpp}
clang-tidy source/**/*.{h,cpp}
```

### 5. Commit with Conventional Format

See [Commit Message Format](#commit-message-format) below.

### 6. Push and Create Pull Request

```bash
git push origin feature/my-new-feature
```

Then open a Pull Request on GitHub with a clear description of your changes.

---

## Commit Message Format

We follow the **Conventional Commits** specification for clear, semantic commit history.

### Format

```
<type>(<scope>): <subject>

<body>

<footer>
```

### Types

- **feat**: New feature
- **fix**: Bug fix
- **docs**: Documentation changes only
- **style**: Code style changes (formatting, whitespace, etc.)
- **refactor**: Code restructuring without behavior change
- **perf**: Performance improvements
- **test**: Adding or updating tests
- **build**: Build system or external dependency changes
- **ci**: CI/CD configuration changes
- **chore**: Maintenance tasks, tooling, etc.

### Examples

```
feat(dsp): add soft/medium/hard knee to gain computer

Implemented three knee modes with configurable transition widths.
Soft knee uses 6dB transition, medium uses 3dB, hard uses 0.1dB.

Closes #42
```

```
fix(ui): correct meter peak hold behavior

Peak hold was resetting too quickly. Changed decay time from 1s to 2s.
```

```
docs(tech): document lookahead latency compensation algorithm

Added mathematical derivation and implementation notes for lookahead
buffer with automated latency reporting to host DAW.
```

```
test(unit): add edge case tests for detector RMS mode

Tests now cover zero input, DC offset, and out-of-range values.
```

---

## Coding Standards

### C++ Style

We use **`.clang-format`** for automatic formatting:

- **Base style**: LLVM
- **Indentation**: 4 spaces (no tabs)
- **Column limit**: 100 characters
- **Brace style**: Allman (braces on new line)

Run before committing:
```bash
clang-format -i source/**/*.{h,cpp}
```

### Naming Conventions

- **Classes**: `PascalCase` (e.g., `GainComputer`, `DetectorEngine`)
- **Functions**: `camelCase` (e.g., `processBlock()`, `setThreshold()`)
- **Variables**: `camelCase` (e.g., `sampleRate`, `attackTime`)
- **Constants**: `UPPER_SNAKE_CASE` (e.g., `MAX_RATIO`, `DEFAULT_THRESHOLD`)
- **Member variables**: `camelCase` with prefix (e.g., `m_buffer` or just `buffer`)

### Documentation

Use **Doxygen-style** comments for public APIs:

```cpp
/**
 * @brief Computes gain reduction based on input level and ratio
 * 
 * @param inputLevel Input signal level in dB
 * @param threshold Compression threshold in dB
 * @param ratio Compression ratio (1.0 to 32.0)
 * @return Gain reduction in dB (always negative or zero)
 */
float computeGainReduction(float inputLevel, float threshold, float ratio);
```

### Real-Time Safety

Audio code **must** be real-time safe:

- ❌ **NO** heap allocations (`new`, `malloc`, `std::vector::push_back`)
- ❌ **NO** locks or mutexes in process() callback
- ❌ **NO** system calls or I/O
- ✅ **YES** pre-allocated buffers
- ✅ **YES** lock-free data structures for parameter updates
- ✅ **YES** `assert()` checks in debug mode

### Test Coverage

All new DSP code should have corresponding unit tests:

```cpp
TEST_CASE("GainComputer calculates correct reduction") {
    GainComputer gc;
    gc.setThreshold(-20.0f);
    gc.setRatio(4.0f);
    
    float reduction = gc.computeGainReduction(-10.0f); // 10dB over threshold
    REQUIRE(reduction == Approx(-7.5f).epsilon(0.01f)); // Expect 7.5dB reduction
}
```

---

## Pull Request Process

### Before Submitting

1. ✅ Ensure all tests pass locally
2. ✅ Run `clang-format` on modified files
3. ✅ Run `clang-tidy` and fix warnings
4. ✅ Update documentation if needed
5. ✅ Add unit tests for new features
6. ✅ Verify no compiler warnings (`-Wall -Wextra`)

### PR Checklist

Your PR description should include:

- [ ] **Summary**: What does this PR do?
- [ ] **Motivation**: Why is this change needed?
- [ ] **Testing**: How was this tested?
- [ ] **Screenshots**: (if UI changes) Before/after images
- [ ] **Breaking changes**: Any API or behavior changes?
- [ ] **Related issues**: Links to issues (e.g., "Closes #42")

### Review Process

1. **Automated checks**: CI will run tests and linters
2. **Code review**: Maintainers will review within 3-5 days
3. **Feedback**: Address any requested changes
4. **Approval**: Once approved, maintainer will merge
5. **Squash**: Commits may be squashed before merge

---

## Development Workflow

### Setting Up Your Environment

#### macOS
```bash
# Install Xcode Command Line Tools
xcode-select --install

# Install CMake (via Homebrew)
brew install cmake

# Install HISE (download from hise.audio)
# Or build from source: https://github.com/christophhart/HISE
```

#### Windows
```bash
# Install Visual Studio 2019 or 2022 (Community Edition)
# Include C++ desktop development workload

# Install CMake from cmake.org
# Or via chocolatey: choco install cmake
```

### Building

```bash
# Create build directory
mkdir build && cd build

# Configure
cmake .. -DCMAKE_BUILD_TYPE=Debug -DPLUGIN_FORMATS=VST3;AU

# Build
cmake --build . --config Debug

# Run tests
ctest --output-on-failure
```

### Hot Reload Development (HISE)

For quick iteration on UI and scripting:

1. Open project in HISE: `hise/xml/EZSqueezeV2.xml`
2. Edit `hise/scripts/ScriptProcessors/EZSqueezeV2/Interface.js`
3. Changes reflect immediately in HISE's interface preview
4. Export when ready for testing in DAW

---

## Feature Requests & Bug Reports

### Filing Issues

Use our GitHub issue templates:

- **Bug Report**: [.github/ISSUE_TEMPLATE/bug_report.md](.github/ISSUE_TEMPLATE/bug_report.md)
- **Feature Request**: [.github/ISSUE_TEMPLATE/feature_request.md](.github/ISSUE_TEMPLATE/feature_request.md)

Include:
- **Clear title**: "GR meter doesn't update in Pro Tools"
- **Steps to reproduce**: Numbered list
- **Expected vs actual behavior**
- **Environment**: OS, DAW, plugin version
- **Screenshots/videos**: If applicable

### Suggesting Enhancements

Before suggesting a new feature:

1. Check existing issues and discussions
2. Consider if it fits project scope
3. Provide use cases and examples
4. Propose implementation approach (if technical)

---

## Areas Where We Need Help

### High Priority
- [ ] DSP algorithm optimization
- [ ] Cross-platform testing (Windows, macOS ARM)
- [ ] Accessibility improvements
- [ ] Documentation (tutorials, examples)
- [ ] Preset creation (diverse styles)

### Medium Priority
- [ ] UI design refinements
- [ ] Performance benchmarking
- [ ] Golden audio test expansion
- [ ] Translation/localization

### Low Priority
- [ ] Additional plugin formats (AAX, VST2)
- [ ] Installer improvements
- [ ] Marketing materials

---

## Questions?

- **Discussions**: [GitHub Discussions](https://github.com/yourusername/EzSqueeze/discussions)
- **Chat**: (Discord/Slack TBD)
- **Email**: support@yourdomain.com

---

## License

By contributing to EzSqueeze, you agree that your contributions will be licensed under the GPL-3.0 License. See [LICENSE](LICENSE) for details.

---

## Recognition

Contributors will be:
- Listed in [README.md](README.md) Credits section
- Mentioned in release notes
- Given a shout-out on social media (if desired)

Thank you for helping make EzSqueeze better! 🍊


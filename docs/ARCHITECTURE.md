# EzSqueeze 代码架构说明

**版本:** 1.0.0  
**最后更新:** 2026 年 3 月

---

## 1. 项目概览

EzSqueeze 是一款基于 **JUCE 7** 的立体声动态压缩器插件，采用 **C++17**、**CMake** 构建，输出 **VST3** 与 **AU**（仅 macOS）。DSP 与 UI 分离：DSP 为头文件实现的模块，便于单测与内联优化。

---

## 2. 目录结构

```
EzSqueeze/
├── CMakeLists.txt              # 主 CMake 配置，FetchContent 拉取 JUCE
├── build.ps1                   # Windows 本地构建脚本（含 -Clean、-Tests）
├── source/
│   ├── plugin/                 # 插件入口与宿主交互
│   │   ├── PluginProcessor.h/cpp   # 主处理器：参数、DSP 调用、延迟上报
│   │   ├── PluginEditor.h/cpp       # 编辑器 UI
│   │   └── Parameters.h             # 参数 ID 与 APVTS 布局
│   └── dsp/                    # 纯 DSP 模块（header-only，ezsqueeze 命名空间）
│       ├── Detector.h              # 峰值/RMS 检测
│       ├── GainComputer.h          # 阈值/比例/拐点
│       ├── EnvelopeFollower.h      # 包络跟随
│       ├── LookaheadBuffer.h       # 前瞻缓冲
│       ├── StereoLink.h            # 立体声链接与 M/S
│       ├── SidechainFilter.h       # 侧链 HPF/LPF
│       ├── AutoMakeup.h            # 自动补偿增益
│       ├── ProgramDependentRelease.h  # 程序相关释放
│       ├── Oversampling.h          # 过采样
│       ├── Saturation.h            # 饱和（Vibe 等用）
│       └── VibeWheel.h             # 风格染色
├── modules/                    # 复合/高级 DSP 模块
│   ├── DualStageCompressor.h   # 双级 FET+Opto
│   ├── CloudGainPreamp.h       # 前级增益与阻抗染色
│   └── TransientSculptor.h     # Snap/Body/De-Snap
├── tests/                      # 单元测试与基准（Catch2）
│   ├── CMakeLists.txt
│   ├── unit/                   # 各 DSP 模块单测
│   └── benchmarks/
├── docs/                       # 文档
│   ├── ARCHITECTURE.md         # 本文件
│   ├── TECH_NOTES.md           # DSP 算法与数学
│   ├── DEPENDENCIES.md         # 构建依赖
│   ├── DESIGN_UI.md            # UI 设计
│   └── USER_MANUAL.md          # 用户手册
├── assets/presets/             # 出厂预设（JSON）
└── hise/                       # HISE 工程（UI 原型，非构建必需）
```

---

## 3. 构建与依赖

- **构建系统:** CMake 3.15+；JUCE 通过 `FetchContent` 自动拉取（默认 tag 7.0.9）。
- **本地构建（Windows）:** 在项目根目录执行：
  - `.\build.ps1` — 配置并编译；
  - `.\build.ps1 -Clean` — 清空 `build/` 后再配置编译；
  - `.\build.ps1 -Tests` — 启用并编译/运行单元测试。
- **依赖摘要:** 见 [docs/DEPENDENCIES.md](DEPENDENCIES.md)。除 CMake 与 C++17 编译器外，无需手动安装 JUCE；测试可选 Catch2（脚本中 `-Tests` 时由 CMake 拉取）。

---

## 4. 插件入口与数据流

- **入口:** `createPluginFilter()`（PluginProcessor.cpp）返回 `EzSqueezeProcessor`。
- **参数:** 全部通过 `juce::AudioProcessorValueTreeState`（APVTS）管理，布局在 `Parameters.h` 的 `createParameterLayout()` 中定义，ID 集中在 `ezsqueeze::ParamID`。
- **延迟:** `getLatencySamples()` = 前瞻采样数 + 过采样延迟，由宿主用于 PDC。

---

## 5. 信号流（与 PluginProcessor 对应）

处理顺序（与 `PluginProcessor::processBlock` 一致）：

1. **Cloud-Gain 前级** — 增益 + 阻抗特性（Silicon/Tube/Transformer）。
2. **过采样（可选）** — 2×/4×/8× 上采样，后续非线性在过采样率下进行。
3. **侧链滤波** — 对检测路径做 HPF/LPF（不改变主路径）。
4. **检测** — 左右声道各自 Peak/RMS，转 dB。
5. **立体声链接** — 左右电平按 link 与 M/S 模式得到用于 GR 的联合电平。
6. **增益计算** — 根据阈值、比例、拐点得到 GR（dB）。
7. **包络跟随** — 对 GR 做 attack/release 平滑；可选程序相关释放。
8. **前瞻** — 对音频路径做延迟，再应用平滑后的 GR。
9. **双级压缩（可选）** — FET 快 + Opto 慢 串联。
10. **Transient Sculptor** — De-Snap 等对检测/路径的整形。
11. **Vibe Wheel** — 饱和与染色。
12. **M/S 解码** — 若为 M/S 模式则解码回 L/R。
13. **下采样** — 若开启了过采样则回到宿主采样率。
14. **干湿混合** — Mix 参数混合干/湿。
15. **补偿增益** — 手动或自动 Makeup。

所有 DSP 模块在 `prepareToPlay` 中按采样率/块大小初始化，在 `releaseResources` 中重置；`processBlock` 内不分配内存、不加锁，保证实时安全。

---

## 6. DSP 模块约定

- **命名空间:** `ezsqueeze`（source/dsp 与 modules 中的类）。
- **接口习惯:**  
  - `setSampleRate(double)` / `prepare(...)` 用于初始化；  
  - `reset()` 用于清状态；  
  - 单采样处理如 `process(float)` 或 `process(float& L, float& R)`。
- **线程:** 仅主线程与音频线程；参数通过 APVTS 的原子读写与音频线程通信，DSP 状态仅音频线程访问。

---

## 7. 测试

- **主工程** 默认不构建测试（`BUILD_TESTS=OFF`）。启用后会在 `add_subdirectory(tests)` 中构建 Catch2 与 `ezsqueeze_unit_tests` / `ezsqueeze_benchmarks`。
- 单测仅依赖 `source/dsp`（及 modules）头文件，不依赖 JUCE，便于快速验证算法。
- 运行方式：`.\build.ps1 -Tests` 或在 `build` 目录执行 `ctest -C Release --output-on-failure`。

---

## 8. 文档索引

| 文档 | 内容 |
|------|------|
| [README.md](../README.md) | 项目介绍、功能、构建与使用概览 |
| [TECH_NOTES.md](TECH_NOTES.md) | DSP 算法、公式与实现细节 |
| [DEPENDENCIES.md](DEPENDENCIES.md) | CMake、编译器、JUCE、可选工具与平台说明 |
| [DESIGN_UI.md](DESIGN_UI.md) | 界面与交互设计 |
| [USER_MANUAL.md](USER_MANUAL.md) | 用户向使用说明 |
| [CONTRIBUTING.md](../CONTRIBUTING.md) | 开发与提交流程 |

---

*EzSqueeze — 专业动态处理，柑橘风格。*

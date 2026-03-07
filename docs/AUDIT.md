# EzSqueeze 代码审计记录

**日期:** 2026 年 3 月  
**范围:** 构建系统、文档、入口与 DSP 调用、单元测试一致性

---

## 审计结论概要

- **构建:** 项目使用 CMake + FetchContent 拉取 JUCE，无 `build.ps1`。已新增 **build.ps1**（支持 `-Clean`、`-Tests`、`-Config`），并更新 README / BUILD.md，便于 Windows 本地一键构建。
- **文档:** 已补充 **docs/ARCHITECTURE.md**（目录结构、信号流、模块约定）、**docs/BUILD.md**（构建步骤与排错）、README 构建小节与文档索引。
- **依赖:** 无需“下载依赖”的额外步骤；CMake 配置时会自动拉取 JUCE。本地需已安装 **CMake 3.15+** 与 **C++17 编译器**（见 DEPENDENCIES.md）。若本机未安装 CMake 或未加入 PATH，运行 `build.ps1` 会提示先安装并配置 PATH。
- **单元测试:** `tests/unit/test_stereo.cpp` 中原先使用已不存在的 API（`processLink` 返回值、`encodeMS`/`decodeMS`）。**StereoLink** 实际为 in-place `processLink(float&, float&)` 与 `encodeMidSide`/`decodeMidSide` 输出参数形式。已修正测试以与当前头文件一致，保证 `BUILD_TESTS=ON` 时可编译通过。

---

## 已修改/新增文件

| 文件 | 变更 |
|------|------|
| **build.ps1** | 新增。Clean/配置/编译/可选测试；检测 CMake 是否在 PATH。 |
| **docs/ARCHITECTURE.md** | 新增。项目结构、信号流、DSP 约定、文档索引。 |
| **docs/BUILD.md** | 新增。前置条件、build.ps1 用法、手动 CMake、常见问题。 |
| **docs/AUDIT.md** | 新增。本审计摘要。 |
| **README.md** | 构建小节改为以 build.ps1 为主，并指向 BUILD.md、ARCHITECTURE、DEPENDENCIES。 |
| **tests/unit/test_stereo.cpp** | 修正 `processLink` 与 M/S 调用，与 `StereoLink.h` 当前 API 一致。 |

---

## 建议后续步骤（维护者）

1. **安装 CMake 并加入 PATH**  
   若尚未安装：[cmake.org](https://cmake.org/download/) 或 `winget install cmake`。安装时勾选 “Add CMake to system PATH”。

2. **验证构建**  
   在项目根目录执行：  
   `.\build.ps1 -Clean`  
   首次会拉取 JUCE，可能需数分钟。成功后产物在 `build\EzSqueeze_artefacts\Release\`。

3. **可选：安装 Ninja**  
   `choco install ninja` 可使本地构建与 CI（Ninja）一致；未安装时脚本会尝试使用 Visual Studio 生成器。

4. **运行测试**  
   `.\build.ps1 -Tests` 或先 `-DBUILD_TESTS=ON` 再 `ctest`（见 BUILD.md）。

---

*本审计未改动 DSP 算法或插件行为，仅涉及构建、文档与测试一致性。*

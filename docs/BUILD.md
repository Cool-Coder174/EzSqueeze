# EzSqueeze 构建指南

本文说明如何在本地配置、拉取依赖并成功编译 EzSqueeze 插件。

---

## 前置要求

- **CMake** 3.15 或更高（[下载](https://cmake.org/download/) 或 `winget install cmake`）
- **C++17 编译器**
  - **Windows:** Visual Studio 2019 或 2022（含“使用 C++ 的桌面开发”）
  - **macOS:** Xcode 12+
- **Ninja（推荐，Windows）：** `choco install ninja` 或 [Ninja  releases](https://github.com/ninja-build/ninja/releases)  
  未安装时，脚本会尝试使用 Visual Studio 生成器。

**JUCE 无需手动安装**：CMake 会通过 `FetchContent` 从 GitHub 拉取 JUCE 7.0.9，首次配置可能需几分钟。

---

## 快速构建（Windows）

在项目根目录打开 PowerShell：

```powershell
# 默认：配置 + 编译（Release）
.\build.ps1

# 清空后重新配置并编译
.\build.ps1 -Clean

# 同时编译并运行单元测试
.\build.ps1 -Clean -Tests
```

构建成功后，插件产物在：

- `build\EzSqueeze_artefacts\Release\`

其中包含 VST3（及 macOS 上的 AU）等格式。

---

## 手动 CMake 构建

若希望自行执行 CMake（例如指定生成器或选项）：

```powershell
# 配置（Ninja，Release）
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release

# 编译
cmake --build build --config Release --parallel
```

**可选：启用测试**

```powershell
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=ON
cmake --build build --config Release --parallel
ctest --test-dir build -C Release --output-on-failure
```

---

## 常见问题

- **“无法将 .\build.ps1 项识别为 cmdlet”**  
  请在 **项目根目录**（包含 `CMakeLists.txt` 和 `build.ps1` 的目录）下运行，且使用 PowerShell。

- **CMake 报错找不到编译器**  
  Windows：从“Developer Command Prompt for VS”或已加载 VS 环境的终端运行；或安装 Ninja 后使用 `-G Ninja`。

- **JUCE 下载很慢或失败**  
  可先手动克隆 JUCE 到本地，再指定路径配置，例如：
  ```powershell
  git clone https://github.com/juce-framework/JUCE.git C:\Libs\JUCE
  cmake -B build -DJUCE_DIR=C:/Libs/JUCE
  ```

- **更多依赖与平台说明**  
  见 [DEPENDENCIES.md](DEPENDENCIES.md)。

---

## 与 CI 一致的做法

GitHub Actions 使用与下面等价的命令（Windows）：

```powershell
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel
```

本地使用 `.\build.ps1`（在已安装 Ninja 时）会与 CI 行为一致。

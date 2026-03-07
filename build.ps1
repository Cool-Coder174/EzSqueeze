<#
.SYNOPSIS
    Build script for EzSqueeze (JUCE plugin). Configures with CMake, fetches JUCE via FetchContent, and builds VST3/AU.

.DESCRIPTION
    Use -Clean to remove the build directory before configuring.
    Use -Tests to also build and run the unit test suite.
    On Windows, Ninja is used if available; otherwise Visual Studio generator is used.

.PARAMETER Clean
    Remove the build directory before configuring (clean build).

.PARAMETER Tests
    Enable BUILD_TESTS and run CTest after building the plugin.

.PARAMETER Config
    Build configuration: Debug or Release (default: Release).

.EXAMPLE
    .\build.ps1
    .\build.ps1 -Clean
    .\build.ps1 -Clean -Tests
#>
param(
    [switch] $Clean,
    [switch] $Tests,
    [ValidateSet("Debug", "Release")]
    [string] $Config = "Release"
)

$ErrorActionPreference = "Stop"
$ProjectRoot = $PSScriptRoot
$BuildDir    = Join-Path $ProjectRoot "build"

# Require CMake on PATH
if (-not (Get-Command cmake -ErrorAction SilentlyContinue)) {
    Write-Error "CMake not found. Install CMake 3.15+ and add it to PATH (see docs/BUILD.md and docs/DEPENDENCIES.md)."
    exit 1
}

# ---- Clean ----
if ($Clean) {
    if (Test-Path $BuildDir) {
        Write-Host "Removing build directory: $BuildDir" -ForegroundColor Yellow
        Remove-Item -Recurse -Force $BuildDir
    }
}

# ---- Ensure build directory exists ----
if (-not (Test-Path $BuildDir)) {
    New-Item -ItemType Directory -Path $BuildDir | Out-Null
}

# ---- Choose generator (Windows: prefer Ninja) ----
$Generator = $null
if ($IsWindows -or $env:OS -match "Windows") {
    $ninjaPath = Get-Command ninja -ErrorAction SilentlyContinue
    if ($ninjaPath) {
        $Generator = "Ninja"
    } else {
        # Try Visual Studio 2022, then 2019
        $vsWhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
        if (Test-Path $vsWhere) {
            $vsPath = & $vsWhere -latest -products * -requires Microsoft.Component.MSBuild -property installationPath 2>$null
            if ($vsPath) {
                $Generator = "Visual Studio 17 2022"
                $Arch = "x64"
                # Fallback to 2019 if 2022 not found
                if (-not $vsPath) {
                    $Generator = "Visual Studio 16 2019"
                    $Arch = "x64"
                }
            }
        }
    }
}

$cmakeArgs = @("-B", $BuildDir)
# Single-config generators (Ninja, Makefiles) need CMAKE_BUILD_TYPE at configure time
if ($Generator -eq "Ninja" -or $Generator -eq $null) {
    $cmakeArgs += "-D", "CMAKE_BUILD_TYPE=$Config"
}
if ($Generator) {
    $cmakeArgs += "-G", $Generator
    if ($Generator -match "Visual Studio" -and $Arch) {
        $cmakeArgs += "-A", $Arch
    }
}
if ($Tests) {
    $cmakeArgs += "-D", "BUILD_TESTS=ON"
}

Write-Host "Configuring CMake (this may download JUCE on first run)..." -ForegroundColor Cyan
Push-Location $ProjectRoot
try {
    & cmake @cmakeArgs
    if ($LASTEXITCODE -ne 0) { throw "CMake configure failed." }

    Write-Host "Building EzSqueeze ($Config)..." -ForegroundColor Cyan
    $buildArgs = @("--build", $BuildDir, "--parallel")
    # Multi-config generators (Visual Studio) need --config at build time
    if ($Generator -match "Visual Studio") {
        $buildArgs += "--config", $Config
    }
    & cmake @buildArgs
    if ($LASTEXITCODE -ne 0) { throw "CMake build failed." }

    if ($Tests) {
        Write-Host "Running tests..." -ForegroundColor Cyan
        & ctest --test-dir $BuildDir --output-on-failure -C $Config
        if ($LASTEXITCODE -ne 0) { throw "Tests failed." }
    }

    Write-Host "Build completed successfully." -ForegroundColor Green
    $artefacts = Join-Path $BuildDir "EzSqueeze_artefacts" $Config
    if (Test-Path $artefacts) {
        Write-Host "Plugin artefacts: $artefacts" -ForegroundColor Gray
    }
} finally {
    Pop-Location
}

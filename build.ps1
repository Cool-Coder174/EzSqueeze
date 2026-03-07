<#
.SYNOPSIS
    Build script for EzSqueeze (JUCE plugin). Configures with CMake, fetches JUCE via FetchContent, and builds VST3/AU.

.DESCRIPTION
    Use -Clean to remove the build directory before configuring.
    Use -Tests to also build and run the unit test suite.
    Use -SkipInstall to build without copying the VST3 to the system folder.
    On Windows, Ninja is used if available; otherwise Visual Studio generator is used.

.PARAMETER Clean
    Remove the build directory before configuring (clean build).

.PARAMETER Tests
    Enable BUILD_TESTS and run CTest after building the plugin.

.PARAMETER Config
    Build configuration: Debug or Release (default: Release).

.PARAMETER SkipInstall
    Do not copy the built VST3 to the system VST3 folder (Windows).

.EXAMPLE
    .\build.ps1
    .\build.ps1 -Clean
    .\build.ps1 -Clean -Tests
    .\build.ps1 -SkipInstall
#>
param(
    [switch] $Clean,
    [switch] $Tests,
    [switch] $SkipInstall,
    [ValidateSet("Debug", "Release")]
    [string] $Config = "Release"
)

$ErrorActionPreference = "Stop"
$ProjectRoot = $PSScriptRoot
$BuildDir    = Join-Path $ProjectRoot "build"
$VST3Dest   = "C:\Program Files\Common Files\VST3"
$ArtifactDir = Join-Path $BuildDir "EzSqueeze_artefacts\$Config"

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
$Arch = $null
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
            } else {
                $Generator = "Visual Studio 16 2019"
                $Arch = "x64"
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
    if (Test-Path $ArtifactDir) {
        Write-Host "Plugin artefacts: $ArtifactDir" -ForegroundColor Gray
        $vst3Dir = Join-Path $ArtifactDir "VST3"
        if (Test-Path $vst3Dir) {
            $vst3Bundle = Get-ChildItem $vst3Dir -Directory -Filter "*.vst3" -ErrorAction SilentlyContinue | Select-Object -First 1
            if ($vst3Bundle) {
                $sizeMB = [math]::Round((Get-ChildItem $vst3Bundle.FullName -Recurse -File | Measure-Object -Property Length -Sum).Sum / 1MB, 2)
                Write-Host ("VST3: {0} ({1} MB)" -f $vst3Bundle.FullName, $sizeMB) -ForegroundColor Gray
            }
        }
    }

    # ---- Install to system VST3 folder (Windows, optional) ----
    if (-not $SkipInstall -and ($IsWindows -or $env:OS -match "Windows")) {
        $vst3Source = Get-ChildItem $BuildDir -Recurse -Directory -Filter "EzSqueeze.vst3" -ErrorAction SilentlyContinue | Select-Object -First 1
        if ($vst3Source) {
            Write-Host ""
            Write-Host "Installing to $VST3Dest ..." -ForegroundColor Green
            $isAdmin = ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
            if (-not $isAdmin) {
                Write-Host "Needs admin rights. Elevating..." -ForegroundColor Yellow
                $cmd = "Copy-Item -Recurse -Force '$($vst3Source.FullName)' '$VST3Dest'; Write-Host 'Installed.' -ForegroundColor Green"
                Start-Process powershell -Verb RunAs -ArgumentList "-NoProfile -Command $cmd"
            } else {
                Copy-Item -Recurse -Force $vst3Source.FullName $VST3Dest
                Write-Host "Installed to $VST3Dest\EzSqueeze.vst3" -ForegroundColor Green
            }
            Write-Host "Rescan plugins in your DAW to load EzSqueeze."
        }
    } elseif ($SkipInstall) {
        Write-Host "Skipping install (-SkipInstall)." -ForegroundColor Gray
    }
} finally {
    Pop-Location
}

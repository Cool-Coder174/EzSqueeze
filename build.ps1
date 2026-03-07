<#
.SYNOPSIS
    Build EzSqueeze VST3 plugin and install to system VST3 folder.
.DESCRIPTION
    Configures with CMake, builds Release, and copies the .vst3 bundle
    to the standard Windows VST3 directory. Run from the repo root.
.EXAMPLE
    .\build.ps1
    .\build.ps1 -Clean
    .\build.ps1 -SkipInstall
#>
param(
    [switch]$Clean,
    [switch]$SkipInstall
)

$ErrorActionPreference = "Stop"
$RepoRoot    = $PSScriptRoot
$BuildDir    = Join-Path $RepoRoot "build"
$VST3Dest    = "C:\Program Files\Common Files\VST3"
$ArtifactDir = Join-Path $BuildDir "EzSqueeze_artefacts\Release\VST3\EzSqueeze.vst3"

Write-Host ""
Write-Host "=== EzSqueeze Build Script ===" -ForegroundColor Cyan

if ($Clean -and (Test-Path $BuildDir)) {
    Write-Host "Removing build directory..." -ForegroundColor Yellow
    Remove-Item -Recurse -Force $BuildDir
}

if (!(Test-Path $BuildDir)) {
    New-Item -ItemType Directory -Path $BuildDir | Out-Null
}

# --- Configure ---
Write-Host ""
Write-Host "[1/3] Configuring CMake..." -ForegroundColor Green
Push-Location $BuildDir
try {
    cmake .. 2>&1 | ForEach-Object { Write-Host $_ }
    if ($LASTEXITCODE -ne 0) { throw "CMake configure failed." }
} finally { Pop-Location }

# --- Build ---
Write-Host ""
Write-Host "[2/3] Building Release..." -ForegroundColor Green
Push-Location $BuildDir
try {
    $output = cmake --build . --config Release 2>&1
    $output | ForEach-Object { Write-Host $_ }
    if ($LASTEXITCODE -ne 0) { throw "Build failed." }

    $warnings = $output | Select-String "warning C\d+"
    if ($warnings) {
        Write-Host ""
        Write-Host "Warnings found:" -ForegroundColor Yellow
        $warnings | ForEach-Object { Write-Host "  $_" -ForegroundColor Yellow }
    } else {
        Write-Host "Build clean - zero warnings." -ForegroundColor Green
    }
} finally { Pop-Location }

# --- Verify ---
if (!(Test-Path $ArtifactDir)) {
    throw "Build artifact not found at $ArtifactDir"
}

$vst3Binary = Get-ChildItem $ArtifactDir -Recurse -Filter "*.vst3" | Select-Object -First 1
$sizeMB = [math]::Round($vst3Binary.Length / 1MB, 2)
Write-Host ""
Write-Host ("Built: {0} ({1} MB)" -f $vst3Binary.FullName, $sizeMB) -ForegroundColor Cyan

# --- Install ---
if ($SkipInstall) {
    Write-Host ""
    Write-Host "Skipping install (-SkipInstall flag set)." -ForegroundColor Yellow
} else {
    Write-Host ""
    Write-Host "[3/3] Installing to $VST3Dest ..." -ForegroundColor Green

    $isAdmin = ([Security.Principal.WindowsPrincipal] `
        [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole(
        [Security.Principal.WindowsBuiltInRole]::Administrator)

    if (!$isAdmin) {
        Write-Host "Needs admin rights. Elevating..." -ForegroundColor Yellow
        $cmd = "Copy-Item -Recurse -Force '$ArtifactDir' '$VST3Dest'; Write-Host 'Installed.' -ForegroundColor Green; pause"
        Start-Process powershell -Verb RunAs -ArgumentList "-NoProfile -Command $cmd"
    } else {
        Copy-Item -Recurse -Force $ArtifactDir $VST3Dest
        Write-Host "Installed to $VST3Dest\EzSqueeze.vst3" -ForegroundColor Green
    }
}

Write-Host ""
Write-Host "=== Done ===" -ForegroundColor Cyan
Write-Host "Rescan plugins in your DAW to load EzSqueeze."
Write-Host ""

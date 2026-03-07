/**
 * @file Parameters.h
 * @brief Defines all EzSqueeze plugin parameters and their layout for APVTS.
 *
 * @copyright 2026 Isaac Hernandez. Licensed under GPL-3.0.
 */

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

namespace ezsqueeze
{

/**
 * @brief Compile-time parameter ID constants referenced by processor and editor.
 */
namespace ParamID
{
    inline constexpr const char* threshold    = "threshold";
    inline constexpr const char* ratio        = "ratio";
    inline constexpr const char* attack       = "attack";
    inline constexpr const char* release      = "release";
    inline constexpr const char* knee         = "knee";
    inline constexpr const char* makeup       = "makeup";
    inline constexpr const char* mix          = "mix";
    inline constexpr const char* detector     = "detector";
    inline constexpr const char* lookahead    = "lookahead";
    inline constexpr const char* stereoLink   = "stereoLink";
    inline constexpr const char* msMode       = "msMode";
    inline constexpr const char* scHPF        = "scHPF";
    inline constexpr const char* scLPF        = "scLPF";
    inline constexpr const char* oversampling = "oversampling";
    inline constexpr const char* ecoMode      = "ecoMode";
    inline constexpr const char* autoRelease  = "autoRelease";
    inline constexpr const char* autoMakeup   = "autoMakeup";
    inline constexpr const char* vibeWheel    = "vibeWheel";
    inline constexpr const char* dualStack    = "dualStack";
    inline constexpr const char* cloudGain    = "cloudGain";
    inline constexpr const char* impedance    = "impedance";
    inline constexpr const char* snap         = "snap";
    inline constexpr const char* body         = "body";
    inline constexpr const char* deSnap       = "deSnap";
    inline constexpr const char* bypass       = "bypass";
}  // namespace ParamID

/**
 * @brief Builds the full APVTS parameter layout for EzSqueeze.
 *
 * Every parameter used by the processor and editor is registered here with its
 * ID string, display name, range, and default value.
 *
 * @return Complete ParameterLayout ready for APVTS construction.
 */
inline juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // --- Core Dynamics ---

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ParamID::threshold, 1},
        "Threshold",
        juce::NormalisableRange<float>(-60.0f, 0.0f, 0.1f),
        -12.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ParamID::ratio, 1},
        "Ratio",
        juce::NormalisableRange<float>(1.0f, 32.0f, 0.1f, 0.5f),
        4.0f,
        juce::AudioParameterFloatAttributes().withLabel(": 1")));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ParamID::attack, 1},
        "Attack",
        juce::NormalisableRange<float>(0.1f, 100.0f, 0.01f, 0.4f),
        4.0f,
        juce::AudioParameterFloatAttributes().withLabel("ms")));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ParamID::release, 1},
        "Release",
        juce::NormalisableRange<float>(5.0f, 500.0f, 0.1f, 0.4f),
        300.0f,
        juce::AudioParameterFloatAttributes().withLabel("ms")));

    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{ParamID::knee, 1},
        "Knee",
        juce::StringArray{"Hard", "Medium", "Soft"},
        0));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ParamID::makeup, 1},
        "Makeup",
        juce::NormalisableRange<float>(0.0f, 24.0f, 0.1f),
        0.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ParamID::mix, 1},
        "Mix",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f),
        100.0f,
        juce::AudioParameterFloatAttributes().withLabel("%")));

    // --- Detection ---

    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{ParamID::detector, 1},
        "Detector",
        juce::StringArray{"Peak", "RMS"},
        0));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ParamID::lookahead, 1},
        "Lookahead",
        juce::NormalisableRange<float>(0.0f, 10.0f, 0.01f),
        0.0f,
        juce::AudioParameterFloatAttributes().withLabel("ms")));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ParamID::stereoLink, 1},
        "Stereo Link",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f),
        100.0f,
        juce::AudioParameterFloatAttributes().withLabel("%")));

    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{ParamID::msMode, 1},
        "M/S Mode",
        false));

    // --- Sidechain Filters ---

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ParamID::scHPF, 1},
        "SC HPF",
        juce::NormalisableRange<float>(20.0f, 400.0f, 1.0f, 0.35f),
        80.0f,
        juce::AudioParameterFloatAttributes().withLabel("Hz")));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ParamID::scLPF, 1},
        "SC LPF",
        juce::NormalisableRange<float>(4000.0f, 16000.0f, 1.0f, 0.35f),
        16000.0f,
        juce::AudioParameterFloatAttributes().withLabel("Hz")));

    // --- Processing Options ---

    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{ParamID::oversampling, 1},
        "Oversampling",
        juce::StringArray{"1x", "2x", "4x", "8x"},
        0));

    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{ParamID::ecoMode, 1},
        "Eco Mode",
        false));

    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{ParamID::autoRelease, 1},
        "Auto Release",
        false));

    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{ParamID::autoMakeup, 1},
        "Auto Makeup",
        false));

    // --- Character & Color ---

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ParamID::vibeWheel, 1},
        "Vibe Wheel",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f),
        0.0f,
        juce::AudioParameterFloatAttributes().withLabel("%")));

    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{ParamID::dualStack, 1},
        "Dual Stack",
        false));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ParamID::cloudGain, 1},
        "Cloud Gain",
        juce::NormalisableRange<float>(0.0f, 30.0f, 0.1f),
        0.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));

    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{ParamID::impedance, 1},
        "Impedance",
        juce::StringArray{"Silicon", "Tube", "Transformer"},
        0));

    // --- Transient Sculptor ---

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ParamID::snap, 1},
        "Snap",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f),
        50.0f,
        juce::AudioParameterFloatAttributes().withLabel("%")));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ParamID::body, 1},
        "Body",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f),
        50.0f,
        juce::AudioParameterFloatAttributes().withLabel("%")));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ParamID::deSnap, 1},
        "De-Snap",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f),
        0.0f,
        juce::AudioParameterFloatAttributes().withLabel("%")));

    // --- Global ---

    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{ParamID::bypass, 1},
        "Bypass",
        false));

    return layout;
}

}  // namespace ezsqueeze

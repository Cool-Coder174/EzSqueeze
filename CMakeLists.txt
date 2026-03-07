##############################################################################
# EzSqueeze — Citrus Dynamics Compressor
# Top-level CMake build configuration
#
# Copyright 2026 Isaac Hernandez. Licensed under GPL-3.0.
##############################################################################

cmake_minimum_required(VERSION 3.15)

project(EzSqueeze
    VERSION   1.0.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD          17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS        OFF)

# ---- Options ---------------------------------------------------------------

option(BUILD_TESTS  "Build the test suite"                      OFF)
option(ENABLE_ASAN  "Enable AddressSanitizer (GCC/Clang only)"  OFF)

# ---- Fetch JUCE ------------------------------------------------------------

include(FetchContent)

FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        7.0.9
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(JUCE)

# ---- Plugin target ----------------------------------------------------------

juce_add_plugin(EzSqueeze
    PRODUCT_NAME                "EzSqueeze"
    COMPANY_NAME                "Isaac Hernandez"
    PLUGIN_MANUFACTURER_CODE    IzHz
    PLUGIN_CODE                 EzSq
    FORMATS                     VST3 AU
    IS_SYNTH                    FALSE
    NEEDS_MIDI_INPUT            FALSE
    NEEDS_MIDI_OUTPUT           FALSE
    IS_MIDI_EFFECT              FALSE
    COPY_PLUGIN_AFTER_BUILD     TRUE
)

juce_generate_juce_header(EzSqueeze)

# ---- Source files -----------------------------------------------------------

target_sources(EzSqueeze
    PRIVATE
        source/plugin/PluginProcessor.cpp
        source/plugin/PluginProcessor.h
        source/plugin/PluginEditor.cpp
        source/plugin/PluginEditor.h
        source/plugin/Parameters.h

        source/dsp/Detector.h
        source/dsp/GainComputer.h
        source/dsp/EnvelopeFollower.h
        source/dsp/LookaheadBuffer.h
        source/dsp/StereoLink.h
        source/dsp/SidechainFilter.h
        source/dsp/AutoMakeup.h
        source/dsp/ProgramDependentRelease.h
        source/dsp/Oversampling.h
        source/dsp/Saturation.h
        source/dsp/VibeWheel.h

        modules/DualStageCompressor.h
        modules/CloudGainPreamp.h
        modules/TransientSculptor.h
)

# ---- Include paths ----------------------------------------------------------

target_include_directories(EzSqueeze
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/source
        ${CMAKE_CURRENT_SOURCE_DIR}/modules
)

# ---- Compile features & definitions ----------------------------------------

target_compile_features(EzSqueeze PRIVATE cxx_std_17)

target_compile_definitions(EzSqueeze
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

# ---- Link JUCE modules -----------------------------------------------------

target_link_libraries(EzSqueeze
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

# ---- Compiler warnings ------------------------------------------------------

if(MSVC)
    target_compile_options(EzSqueeze PRIVATE /W4)
else()
    target_compile_options(EzSqueeze PRIVATE -Wall -Wextra -Wpedantic)
endif()

# ---- AddressSanitizer -------------------------------------------------------

if(ENABLE_ASAN AND NOT MSVC)
    target_compile_options(EzSqueeze PRIVATE -fsanitize=address -fno-omit-frame-pointer)
    target_link_options(EzSqueeze    PRIVATE -fsanitize=address)
endif()

# ---- Tests ------------------------------------------------------------------

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

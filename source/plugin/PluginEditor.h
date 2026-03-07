/**
 * @file PluginEditor.h
 * @brief Placeholder editor for EzSqueeze (production UI is in HISE).
 *
 * @copyright 2026 Isaac Hernandez. Licensed under GPL-3.0.
 */

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include "PluginProcessor.h"

/**
 * @class EzSqueezeEditor
 * @brief Minimal stand-in editor; the full UI ships via the HISE export.
 */
class EzSqueezeEditor : public juce::AudioProcessorEditor
{
public:
    explicit EzSqueezeEditor(EzSqueezeProcessor& processor);
    ~EzSqueezeEditor() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    EzSqueezeProcessor& processorRef_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EzSqueezeEditor)
};

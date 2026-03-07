/**
 * @file PluginEditor.cpp
 * @brief Placeholder editor implementation for EzSqueeze.
 *
 * @copyright 2026 Isaac Hernandez. Licensed under GPL-3.0.
 */

#include "PluginEditor.h"

EzSqueezeEditor::EzSqueezeEditor(EzSqueezeProcessor& processor)
    : AudioProcessorEditor(processor), processorRef_(processor)
{
    setSize(900, 650);
}

void EzSqueezeEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff1a1a2e));

    g.setColour(juce::Colour(0xffef932c));
    g.setFont(juce::FontOptions(28.0f));
    g.drawText("EzSqueeze", getLocalBounds().removeFromTop(80), juce::Justification::centred);

    g.setColour(juce::Colours::white.withAlpha(0.6f));
    g.setFont(juce::FontOptions(16.0f));
    g.drawText(
        "Use HISE for full UI",
        getLocalBounds().reduced(20),
        juce::Justification::centred);
}

void EzSqueezeEditor::resized()
{
}

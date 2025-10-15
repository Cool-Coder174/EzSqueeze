/**
 * @file TempoSync.cpp
 * @brief Implementation of tempo synchronization
 */

#include "TempoSync.h"

namespace EzSqueeze {
namespace DSP {

// Static member initialization
float TempoSync::s_bpm = 120.0f;

void TempoSync::setBPM(float bpm)
{
    s_bpm = std::clamp(bpm, 20.0f, 300.0f);
}

float TempoSync::noteToMs(NoteDivision division)
{
    switch (division)
    {
        case NoteDivision::Whole:
            return calculateMs(1.0f);
        case NoteDivision::Half:
            return calculateMs(2.0f);
        case NoteDivision::Quarter:
            return calculateMs(4.0f);
        case NoteDivision::Eighth:
            return calculateMs(8.0f);
        case NoteDivision::Sixteenth:
            return calculateMs(16.0f);
        case NoteDivision::ThirtySecond:
            return calculateMs(32.0f);
        case NoteDivision::SixtyFourth:
            return calculateMs(64.0f);
        case NoteDivision::HalfDotted:
            return calculateMs(2.0f) * 1.5f;
        case NoteDivision::QuarterDotted:
            return calculateMs(4.0f) * 1.5f;
        case NoteDivision::EighthDotted:
            return calculateMs(8.0f) * 1.5f;
        case NoteDivision::HalfTriplet:
            return calculateMs(2.0f) / 1.5f;
        case NoteDivision::QuarterTriplet:
            return calculateMs(4.0f) / 1.5f;
        case NoteDivision::EighthTriplet:
            return calculateMs(8.0f) / 1.5f;
        default:
            return calculateMs(4.0f);  // Default to quarter note
    }
}

const char* TempoSync::getNoteName(NoteDivision division)
{
    switch (division)
    {
        case NoteDivision::Whole: return "1/1";
        case NoteDivision::Half: return "1/2";
        case NoteDivision::Quarter: return "1/4";
        case NoteDivision::Eighth: return "1/8";
        case NoteDivision::Sixteenth: return "1/16";
        case NoteDivision::ThirtySecond: return "1/32";
        case NoteDivision::SixtyFourth: return "1/64";
        case NoteDivision::HalfDotted: return "1/2D";
        case NoteDivision::QuarterDotted: return "1/4D";
        case NoteDivision::EighthDotted: return "1/8D";
        case NoteDivision::HalfTriplet: return "1/2T";
        case NoteDivision::QuarterTriplet: return "1/4T";
        case NoteDivision::EighthTriplet: return "1/8T";
        default: return "1/4";
    }
}

float TempoSync::calculateMs(float beatsPerNote)
{
    // Formula: ms = (60,000 / BPM) * (4 / beatsPerNote)
    // where 4 represents quarter note
    const float msPerBeat = 60000.0f / s_bpm;
    const float beats = 4.0f / beatsPerNote;
    return msPerBeat * beats;
}

} // namespace DSP
} // namespace EzSqueeze


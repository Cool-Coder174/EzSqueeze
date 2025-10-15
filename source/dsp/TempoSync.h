/**
 * @file TempoSync.h
 * @brief Tempo synchronization for time-based parameters
 * 
 * Converts musical note divisions to milliseconds based on BPM,
 * allowing attack/release to be synced to tempo.
 * 
 * @author Isaac Hernandez
 * @date October 2025
 */

#pragma once

#include <cmath>
#include <algorithm>

namespace EzSqueeze {
namespace DSP {

/**
 * @brief Musical note divisions for tempo sync
 */
enum class NoteDivision
{
    Whole = 0,      ///< 1/1 (whole note)
    Half,           ///< 1/2
    Quarter,        ///< 1/4
    Eighth,         ///< 1/8
    Sixteenth,      ///< 1/16
    ThirtySecond,   ///< 1/32
    SixtyFourth,    ///< 1/64
    HalfDotted,     ///< 1/2 dotted
    QuarterDotted,  ///< 1/4 dotted
    EighthDotted,   ///< 1/8 dotted
    HalfTriplet,    ///< 1/2 triplet
    QuarterTriplet, ///< 1/4 triplet
    EighthTriplet   ///< 1/8 triplet
};

/**
 * @class TempoSync
 * @brief Tempo synchronization utility
 * 
 * Converts musical note divisions to time values based on BPM.
 * Useful for syncing attack/release times to tempo.
 * 
 * RT-Safe: Yes (pure computation)
 * Complexity: O(1)
 */
class TempoSync
{
public:
    /**
     * @brief Set tempo in BPM
     * @param bpm Beats per minute (20-300)
     */
    static void setBPM(float bpm);

    /**
     * @brief Convert note division to milliseconds
     * @param division Musical note division
     * @return Time in milliseconds
     */
    static float noteToMs(NoteDivision division);

    /**
     * @brief Get current BPM
     * @return Beats per minute
     */
    static float getBPM() { return s_bpm; }

    /**
     * @brief Get readable name for note division
     * @param division Note division
     * @return Human-readable string (e.g., "1/4", "1/8T")
     */
    static const char* getNoteName(NoteDivision division);

private:
    static float s_bpm;

    /**
     * @brief Calculate milliseconds for a note division
     * @param beatsPerNote How many beats per note (e.g., 4 for quarter note)
     * @return Time in ms
     */
    static float calculateMs(float beatsPerNote);
};

} // namespace DSP
} // namespace EzSqueeze


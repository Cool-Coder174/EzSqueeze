/**
 * @file StereoProcessor.h
 * @brief Stereo linking and M/S processing for compression
 * 
 * Provides stereo link control and Mid/Side encoding/decoding
 * for independent M/S compression.
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
 * @class StereoLink
 * @brief Stereo channel linking for unified compression
 * 
 * Blends between independent L/R detection (0%) and fully
 * linked stereo detection (100%) to maintain stereo image
 * while preventing channel pumping.
 * 
 * RT-Safe: Yes (pure computation)
 * Complexity: O(1)
 */
class StereoLink
{
public:
    /**
     * @brief Set stereo link amount
     * @param linkAmount Link percentage (0.0 = independent, 1.0 = fully linked)
     */
    void setLinkAmount(float linkAmount);

    /**
     * @brief Compute linked level from L/R levels
     * @param leftLevel Left channel level in dB
     * @param rightLevel Right channel level in dB
     * @param outLeft Output linked level for left (dB)
     * @param outRight Output linked level for right (dB)
     */
    void process(float leftLevel, float rightLevel, 
                 float& outLeft, float& outRight) const;

private:
    float m_linkAmount = 1.0f;  // Default: fully linked
};

/**
 * @class MSProcessor
 * @brief Mid/Side encoding and decoding
 * 
 * Converts L/R stereo to M/S for independent processing of
 * center (Mid) and sides (Side) content, then decodes back
 * to L/R.
 * 
 * RT-Safe: Yes (pure computation)
 * Complexity: O(1)
 */
class MSProcessor
{
public:
    /**
     * @brief Encode L/R to M/S
     * @param left Left channel sample
     * @param right Right channel sample
     * @param outMid Mid (center) component
     * @param outSide Side (difference) component
     */
    static void encode(float left, float right, 
                      float& outMid, float& outSide);

    /**
     * @brief Decode M/S to L/R
     * @param mid Mid (center) component
     * @param side Side (difference) component
     * @param outLeft Left channel sample
     * @param outRight Right channel sample
     */
    static void decode(float mid, float side,
                      float& outLeft, float& outRight);
};

} // namespace DSP
} // namespace EzSqueeze


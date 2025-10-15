/**
 * @file StereoProcessor.cpp
 * @brief Implementation of stereo processing utilities
 */

#include "StereoProcessor.h"

namespace EzSqueeze {
namespace DSP {

// ============================================================================
// StereoLink Implementation
// ============================================================================

void StereoLink::setLinkAmount(float linkAmount)
{
    m_linkAmount = std::clamp(linkAmount, 0.0f, 1.0f);
}

void StereoLink::process(float leftLevel, float rightLevel,
                         float& outLeft, float& outRight) const
{
    // Compute maximum level for full link
    const float linkedLevel = std::max(leftLevel, rightLevel);
    
    // Blend between independent and linked
    outLeft = leftLevel + m_linkAmount * (linkedLevel - leftLevel);
    outRight = rightLevel + m_linkAmount * (linkedLevel - rightLevel);
}

// ============================================================================
// MSProcessor Implementation
// ============================================================================

void MSProcessor::encode(float left, float right,
                         float& outMid, float& outSide)
{
    // Mid/Side encoding
    // Mid = (L + R) / 2  (mono sum, center content)
    // Side = (L - R) / 2  (stereo difference, width)
    
    outMid = (left + right) * 0.5f;
    outSide = (left - right) * 0.5f;
}

void MSProcessor::decode(float mid, float side,
                         float& outLeft, float& outRight)
{
    // M/S decoding (inverse of encoding)
    // L = Mid + Side
    // R = Mid - Side
    
    outLeft = mid + side;
    outRight = mid - side;
}

} // namespace DSP
} // namespace EzSqueeze


#pragma once

#include <cmath>
#include <algorithm>

namespace ezsqueeze
{

/**
 * @brief Stereo link percentage blending and Mid/Side encode/decode.
 *
 * Controls how much the left and right compressor sidechains are
 * linked (0 = fully independent, 1 = fully linked using max),
 * and provides Mid/Side matrix utilities.
 */
class StereoLink
{
public:
    /**
     * @brief Set the stereo link amount.
     * @param amount  0.0 = independent, 1.0 = fully linked.
     */
    void setLinkAmount(float amount)
    {
        linkAmount_ = std::clamp(amount, 0.0f, 1.0f);
    }

    /**
     * @brief Enable or disable Mid/Side processing mode.
     * @param enabled  True for M/S, false for L/R.
     */
    void setMSMode(bool enabled)
    {
        msMode_ = enabled;
    }

    /**
     * @brief Blend left and right detected levels based on link amount.
     *
     * At 0 %: levels are unchanged (independent).
     * At 100 %: both levels become max(left, right) (fully linked).
     *
     * @param[in,out] leftLevel   Left channel detected level.
     * @param[in,out] rightLevel  Right channel detected level.
     */
    void processLink(float& leftLevel, float& rightLevel) const
    {
        float linked = std::max(leftLevel, rightLevel);
        leftLevel  = leftLevel  + linkAmount_ * (linked - leftLevel);
        rightLevel = rightLevel + linkAmount_ * (linked - rightLevel);
    }

    /**
     * @brief Encode left/right to Mid/Side.
     * @param L  Left input.
     * @param R  Right input.
     * @param[out] M  Mid output:  (L + R) * 0.5
     * @param[out] S  Side output: (L - R) * 0.5
     */
    static void encodeMidSide(float L, float R, float& M, float& S)
    {
        M = (L + R) * 0.5f;
        S = (L - R) * 0.5f;
    }

    /**
     * @brief Decode Mid/Side back to left/right.
     * @param M  Mid input.
     * @param S  Side input.
     * @param[out] L  Left output:  M + S
     * @param[out] R  Right output: M - S
     */
    static void decodeMidSide(float M, float S, float& L, float& R)
    {
        L = M + S;
        R = M - S;
    }

    /** @brief Return whether M/S mode is active. */
    bool isMSMode() const { return msMode_; }

    /** @brief Return the current link amount. */
    float getLinkAmount() const { return linkAmount_; }

private:
    float linkAmount_ = 1.0f;
    bool msMode_ = false;
};

} // namespace ezsqueeze

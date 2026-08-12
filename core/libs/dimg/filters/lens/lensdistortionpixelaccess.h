/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-12-27
 * Description : access pixels method for lens distortion algorithm.
 *
 * SPDX-FileCopyrightText: 2004-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "dimg.h"
#include "digikam_export.h"

namespace Digikam
{

static const int LensDistortionPixelAccessRegions = 20;
static const int LensDistortionPixelAccessWidth   = 40;
static const int LensDistortionPixelAccessHeight  = 20;
static const int LensDistortionPixelAccessXOffset = 3;
static const int LensDistortionPixelAccessYOffset = 3;

/**
 * LensDistortionPixelAccess class: solving the eternal problem: random, cubic-interpolated,
 * sub-pixel coordinate access to an image.
 * Assuming that accesses are at least slightly coherent,
 * LensDistortionPixelAccess keeps LensDistortionPixelAccessRegions buffers, each containing a
 * LensDistortionPixelAccessWidth x LensDistortionPixelAccessHeight region of pixels.
 * Buffer[0] is always checked first, so move the last accessed
 * region into that position.
 * When a request arrives which is outside all the regions,
 * get a new region.
 * The new region is placed so that the requested pixel is positioned
 * at [LensDistortionPixelAccessXOffset, LensDistortionPixelAccessYOffset] in the region.
 */
class DIGIKAM_EXPORT LensDistortionPixelAccess
{

public:

    explicit LensDistortionPixelAccess(DImg* srcImage);
    ~LensDistortionPixelAccess();

    void pixelAccessGetCubic(double srcX, double srcY, double brighten, uchar* dst);

protected:

    inline uchar* pixelAccessAddress(int i, int j);
    void pixelAccessSelectRegion(int n);
    void pixelAccessDoEdge(int i, int j);
    void pixelAccessReposition(int xInt, int yInt);
    void cubicInterpolate(uchar* src, int rowStride, uchar* dst,
                          bool sixteenBit, double dx, double dy, double brighten);

private:

    DImg* m_image                                       = nullptr;

    DImg* m_buffer[LensDistortionPixelAccessRegions]    = { nullptr };

    int   m_width                                       = 0;
    int   m_height                                      = 0;
    int   m_depth                                       = 8;
    int   m_imageWidth                                  = 0;
    int   m_imageHeight                                 = 0;
    bool  m_sixteenBit                                  = false;
    int   m_tileMinX[LensDistortionPixelAccessRegions]  = { 0 };
    int   m_tileMaxX[LensDistortionPixelAccessRegions]  = { 0 };
    int   m_tileMinY[LensDistortionPixelAccessRegions]  = { 0 };
    int   m_tileMaxY[LensDistortionPixelAccessRegions]  = { 0 };

private:

    /// @note disabled
    LensDistortionPixelAccess(const LensDistortionPixelAccess&)            = delete;
    LensDistortionPixelAccess& operator=(const LensDistortionPixelAccess&) = delete;
};

} // namespace Digikam

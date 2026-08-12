/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-03-02
 * Description : DColor methods for composing
 *
 * SPDX-FileCopyrightText: 2006-2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "dcolor.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT DColorComposer
{
public:

    /**
     * @brief The available rules to combine src and destination color.
     *
     * For the Porter-Duff rules, the formula is
     *     component = (source * fs + destination * fd)
     * where
     *     fs, fd according to the following table with
     *     sa = source alpha,
     *     da = destination alpha:
     *
     *     None          fs: sa      fd: 1.0-sa
     *     Clear         fs: 0.0     fd: 0.0
     *     Src           fs: 1.0     fd: 0.0
     *     Src Over      fs: 1.0     fd: 1.0-sa
     *     Dst Over      fs: 1.0-da  fd: 1.0
     *     Src In        fs: da      fd: 0.0
     *     Dst In        fs: 0.0     fd: sa
     *     Src Out       fs: 1.0-da  fd: 0.0
     *     Dst Out       fs: 0.0     fd: 1.0-sa
     *
     *     Src Atop      fs: da      fd: 1.0-sa
     *     Dst Atop      fs: 1.0-da  fd: sa
     *     Xor           fs: 1.0-da  fd: 1.0-sa
     *
     *     None is the default, classical blending mode, a "Src over" simplification:
     *      Blend non-premultiplied RGBA data "src over" a fully opaque background.
     *     Src is the painter's algorithm.
     *     All other operations require premultiplied colors.
     *     The documentation of java.awt.AlphaComposite (Java 1.5)
     *     provides a good introduction and documentation on Porter Duff.
     */
    enum CompositingOperation
    {
        PorterDuffNone,
        PorterDuffClear,
        PorterDuffSrc,
        PorterDuffSrcOver,
        PorterDuffDstOver,
        PorterDuffSrcIn,
        PorterDuffDstIn,
        PorterDuffSrcOut,
        PorterDuffDstOut,
        PorterDuffSrcAtop,
        PorterDuffDstAtop,
        PorterDuffXor
    };

    enum MultiplicationFlags
    {
        NoMultiplication = 0x00,
        PremultiplySrc   = 0x01,
        PremultiplyDst   = 0x02,
        DemultiplyDst    = 0x04,

        MultiplicationFlagsDImg                     = PremultiplySrc | PremultiplyDst | DemultiplyDst,
        MultiplicationFlagsPremultipliedColorOnDImg = PremultiplyDst | DemultiplyDst
    };

    /**
     * @brief Retrieve a DColorComposer object for one of the predefined rules.
     * The object needs to be deleted by the caller.
     */
    static DColorComposer* getComposer(CompositingOperation rule);

    /**
     * @brief Carry out the actual composition process.
     * @param src and @param dest are composed and the result is written to the destination.
     * No pre-/demultiplication is done by this method, use the other overloaded
     * methods, which call this method, if you need  pre- or demultiplication
     * (you need it if any of the colors are read from or written to a DImg).
     *
     * If you just pass the object to a DImg method, you do not need to call this.
     * Call this function if you want to compose two colors.
     * Implement this function if you create a custom DColorComposer.
     *
     * The bit depth of source and destination color must be identical.
     */
    virtual void compose(DColor& dest, DColor& src) = 0;

    /**
     * @brief Compose the two colors by calling compose(dest, src).
     * Pre- and de-multiplication operations are done as specified.
     * For PorterDuff operations except PorterDuffNone, you need
     *
     * - PremultiplySrc    if src is not premultiplied (read from a DImg)
     * - PremultiplyDst    if dst is not premultiplied (read from a DImg)
     * - DemultiplyDst     if dst will be written to non-premultiplied data (a DImg)
     */
    virtual void compose(DColor& dest, DColor& src, MultiplicationFlags multiplicationFlags);

    DColorComposer()          = default;
    virtual ~DColorComposer() = default;

private:

    Q_DISABLE_COPY(DColorComposer)
};

} // namespace Digikam

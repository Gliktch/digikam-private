/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-05-25
 * Description : Raindrop threaded image filter.
 *
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2010      by Martin Klapetek <martin dot klapetek at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "digikam_export.h"
#include "dimgthreadedfilter.h"
#include "digikam_globals.h"
#include "randomnumbergenerator.h"

class QRect;

namespace Digikam
{

class DIGIKAM_EXPORT RainDropFilter : public DImgThreadedFilter
{
    Q_OBJECT

private:

    class Q_DECL_HIDDEN Args
    {
    public:

        Args() = default;

        uint   start        = 0;
        uint   stop         = 0;
        DImg*  orgImage     = nullptr;
        DImg*  destImage    = nullptr;
        int    MinDropSize  = 0;
        int    MaxDropSize  = 0;
        int    Coeff        = 0;
        bool   bLimitRange  = false;
        uchar* pStatusBits  = nullptr;
    };

public:

    explicit RainDropFilter(QObject* const parent = nullptr);
    explicit RainDropFilter(DImg* const orgImage,
                            QObject* const parent = nullptr,
                            int drop = 80,
                            int amount = 150,
                            int coeff = 30,
                            const QRect& selection = QRect(0, 0, 0, 0));
    ~RainDropFilter()                                                         override;

    static QString          FilterIdentifier()
    {
        return QLatin1String("digikam:RainDropFilter");
    }

    static QString          DisplayableName();

    static QList<int>       SupportedVersions()
    {
        return QList<int>() << 1;
    }

    static int              CurrentVersion()
    {
        return 1;
    }

    QString         filterIdentifier()                                  const override
    {
        return FilterIdentifier();
    }

    FilterAction    filterAction()                                            override;

    void                    readParameters(const FilterAction& action)        override;

private:

    void filterImage()                                                        override;

    void rainDropsImage(DImg* const orgImage, DImg* const destImage, int MinDropSize, int MaxDropSize,
                        int Amount, int Coeff, bool bLimitRange, int progressMin, int progressMax);

    void rainDropsImageMultithreaded(const Args& prm);

    bool CreateRainDrop(uchar* const pBits, int Width, int Height, bool sixteenBit, int bytesDepth,
                        uchar* const pResBits, uchar* const pStatusBits,
                        int X, int Y, int DropSize, double Coeff, bool bLimitRange);

    bool CanBeDropped(int Width, int Height, uchar* const pStatusBits, int X, int Y, int DropSize, bool bLimitRange);

    bool SetDropStatusBits(int Width, int Height, uchar* const pStatusBits, int X, int Y, int DropSize);

    /**
     * A color is represented in RGB value (e.g. 0xFFFFFF is white color).
     * But R, G and B values has 256 values to be used so, this function analyzes
     * the value and limits to this range.
     */
    inline int  limitValues8(int ColorValue);

    inline int  limitValues16(int ColorValue);

    inline bool isInside (int Width, int Height, int X, int Y);

    inline int  pixelOffset(int Width, int X, int Y, int bytesDepth);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam

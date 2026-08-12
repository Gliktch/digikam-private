/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-05-25
 * Description : Oil Painting threaded image filter.
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

namespace Digikam
{

class DIGIKAM_EXPORT OilPaintFilter : public DImgThreadedFilter
{
    Q_OBJECT

public:

    explicit OilPaintFilter(QObject* const parent = nullptr);
    explicit OilPaintFilter(DImg* const orgImage, QObject* const parent = nullptr,
                            int brushSize = 1, int smoothness = 30);
    ~OilPaintFilter()                                                         override;

    static QString          FilterIdentifier()
    {
        return QLatin1String("digikam:OilPaintFilter");
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
    void oilPaintImageMultithreaded(uint start, uint stop);
    DColor MostFrequentColor(const DImg& src, int X, int Y, int Radius, int Intensity,
                             uchar* intensityCount, uint* averageColorR,
                             uint* averageColorG, uint* averageColorB);
    inline double GetIntensity(uint Red, uint Green, uint Blue);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam

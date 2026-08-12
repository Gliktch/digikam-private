/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-07-18
 * Description : Free rotation threaded image filter.
 *
 * SPDX-FileCopyrightText: 2004-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2010 by Andi Clemens <andi dot clemens at gmail dot com>
 * SPDX-FileCopyrightText: 2010      by Martin Klapetek <martin dot klapetek at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QSize>
#include <QColor>

// Local includes

#include "digikam_export.h"
#include "dimgthreadedfilter.h"

namespace Digikam
{

class DIGIKAM_EXPORT FreeRotationContainer
{
public:

    enum AutoCropTypes
    {
        NoAutoCrop = 0,
        WidestArea,
        LargestArea
    };

public:

    FreeRotationContainer()  = default;
    ~FreeRotationContainer() = default;

public:

    bool   antiAlias        = true;

    int    autoCrop         = NoAutoCrop;
    int    orgW             = 0;
    int    orgH             = 0;

    double angle            = 0.0;

    QSize  newSize;

    QColor backgroundColor  = Qt::black;
};

// -----------------------------------------------------------------------------------------

class DIGIKAM_EXPORT FreeRotationFilter : public DImgThreadedFilter
{
    Q_OBJECT

public:

    explicit FreeRotationFilter(QObject* const parent = nullptr);
    explicit FreeRotationFilter(DImg* const orgImage, QObject* const parent = nullptr,
                                const FreeRotationContainer& settings = FreeRotationContainer());

    ~FreeRotationFilter()                                                     override;

    QSize getNewSize() const;

    static double calculateAngle(int x1, int y1, int x2, int y2);
    static double calculateAngle(const QPoint& p1, const QPoint& p2);

    static QString          FilterIdentifier()
    {
        return QLatin1String("digikam:FreeRotationFilter");
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

    void        filterImage()                                                 override;
    inline int  setPosition (int Width, int X, int Y);
    inline bool isInside (int Width, int Height, int X, int Y);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam

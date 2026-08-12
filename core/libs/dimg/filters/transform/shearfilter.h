/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-07-18
 * Description : Shear tool threaded image filter.
 *
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include "digikam_globals.h"

namespace Digikam
{

class DIGIKAM_EXPORT ShearFilter : public DImgThreadedFilter
{
    Q_OBJECT

public:

    explicit ShearFilter(QObject* const parent = nullptr);
    explicit ShearFilter(DImg* const orgImage, QObject* const parent = nullptr, float hAngle = 0.0, float vAngle = 0.0,
                         bool antialiasing = true, const QColor& backgroundColor = Qt::black, int orgW = 0, int orgH = 0);
    ~ShearFilter()                                                            override;

    QSize getNewSize() const;

    static QString          FilterIdentifier()
    {
        return QLatin1String("digikam:ShearFilter");
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

    inline int setPosition (int Width, int X, int Y)
    {
        return (Y*Width*4 + 4*X);
    };

    inline bool isInside (int Width, int Height, int X, int Y)
    {
        bool bIsWOk = ((X < 0) ? false : (X >= Width ) ? false : true);
        bool bIsHOk = ((Y < 0) ? false : (Y >= Height) ? false : true);

        return (bIsWOk && bIsHOk);
    };

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam

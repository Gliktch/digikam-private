/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-05-25
 * Description : border threaded image filter.
 *
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2009-2010 by Andi Clemens <andi dot clemens at gmail dot com>
 * SPDX-FileCopyrightText: 2010      by Martin Klapetek <martin dot klapetek at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QImage>

// Local includes

#include "dimgthreadedfilter.h"
#include "digikam_globals.h"
#include "bordercontainer.h"

namespace Digikam
{

class DIGIKAM_EXPORT BorderFilter : public DImgThreadedFilter
{
    Q_OBJECT

public:

    /**
     * Constructor using settings to preserve aspect ratio of image.
     */
    explicit BorderFilter(QObject* const parent = nullptr);
    explicit BorderFilter(DImg* orgImage, QObject* const parent = nullptr,
                          const BorderContainer& settings = BorderContainer());
    ~BorderFilter()                                           override;

    static QString FilterIdentifier()
    {
        return QLatin1String("digikam:BorderFilter");
    }

    static QString DisplayableName();

    static QList<int> SupportedVersions()
    {
        return QList<int>() << 1;
    }

    static int CurrentVersion()
    {
        return 1;
    }

    QString filterIdentifier()                          const override
    {
        return FilterIdentifier();
    }

    FilterAction filterAction()                               override;
    void readParameters(const FilterAction& action)           override;

private:

    void filterImage()                                        override;

    /**
     * Methods to preserve aspect ratio of image.
     */
    void solid(const DImg& src, DImg& dest, const DColor& fg, int borderWidth);
    void niepce(const DImg& src, DImg& dest, const DColor& fg, int borderWidth,
                const DColor& bg, int lineWidth);
    void bevel(const DImg& src, DImg& dest, const DColor& topColor,
               const DColor& btmColor, int borderWidth);
    void pattern(const DImg& src, DImg& dest, int borderWidth, const DColor& firstColor,
                 const DColor& secondColor, int firstWidth, int secondWidth);

    /**
     * Methods to not-preserve aspect ratio of image.
     */
    void solid2(const DImg& src, DImg& dest, const DColor& fg, int borderWidth);
    void niepce2(const DImg& src, DImg& dest, const DColor& fg, int borderWidth,
                 const DColor& bg, int lineWidth);
    void bevel2(const DImg& src, DImg& dest, const DColor& topColor,
                const DColor& btmColor, int borderWidth);
    void pattern2(const DImg& src, DImg& dest, int borderWidth, const DColor& firstColor,
                  const DColor& secondColor, int firstWidth, int secondWidth);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam

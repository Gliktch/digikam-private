/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-03-27
 * Description : Threaded image filter to fix hot pixels
 *
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2005-2006 by Unai Garro <ugarro at users dot sourceforge dot net>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QList>
#include <QImage>
#include <QObject>
#include <QRect>
#include <QString>

// Local includes

#include "digikam_export.h"
#include "dimgthreadedfilter.h"
#include "hotpixelprops.h"
#include "hotpixelsweights.h"
#include "hotpixelcontainer.h"

using namespace Digikam;

namespace Digikam
{

class DIGIKAM_EXPORT HotPixelFixer : public DImgThreadedFilter
{
    Q_OBJECT

public:

    explicit HotPixelFixer(QObject* const parent = nullptr);
    explicit HotPixelFixer(DImg* const orgImage,
                           QObject* const parent,
                           const HotPixelContainer& settings);
    ~HotPixelFixer() override;

    static QString          FilterIdentifier()
    {
        return QLatin1String("digikam:HotPixelFilter");
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

    void readParameters(const FilterAction& action)  override;

    QString         filterIdentifier() const override
    {
        return FilterIdentifier();
    }

    Digikam::FilterAction filterAction()     override;

private:

    void filterImage()                       override;

    void interpolate(DImg& img,
                     const HotPixelProps& hp,
                     int method);

    void weightPixels(DImg& img,
                      const HotPixelProps& px,
                      int method,
                      HotPixelContainer::Direction dir,
                      int maxComponent);

    inline bool validPoint(const DImg& img, const QPoint& p)
    {
        return (
                (p.x() >= 0)                 &&
                (p.y() >= 0)                 &&
                (p.x() < (long) img.width()) &&
                (p.y() < (long) img.height())
               );
    };


private:

    QList<HotPixelsWeights> m_weightList;

    HotPixelContainer       m_settings;
};

} // namespace Digikam

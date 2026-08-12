/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-17-07
 * Description : A Unsharp Mask threaded image filter.
 *
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2009      by Matthias Welwarsky <matze at welwarsky dot de>
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

class DIGIKAM_EXPORT UnsharpMaskFilter : public DImgThreadedFilter
{
    Q_OBJECT

public:

    explicit UnsharpMaskFilter(QObject* const parent = nullptr);
    explicit UnsharpMaskFilter(DImg* const orgImage,
                               QObject* const parent = nullptr,
                               double radius = 1.0,
                               double amount = 1.0,
                               double threshold = 0.05,
                               bool luma=false);

    ~UnsharpMaskFilter()                                                      override;

    static QString          FilterIdentifier()
    {
        return QLatin1String("digikam:UnsharpMaskFilter");
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
    void unsharpMaskMultithreaded(uint start, uint stop, uint y);

private:

    double m_radius     = 1.0;
    double m_amount     = 1.0;
    double m_threshold  = 0.05;
    bool   m_luma       = false;
};

} // namespace Digikam

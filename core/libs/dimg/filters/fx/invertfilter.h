/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-17-07
 * Description : An invert image threaded image filter.
 *
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2010      by Martin Klapetek <martin dot klapetek at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "digikam_export.h"
#include "dimgthreadedfilter.h"

namespace Digikam
{

class DIGIKAM_EXPORT InvertFilter : public DImgThreadedFilter
{
    Q_OBJECT

public:

    explicit InvertFilter(QObject* const parent = nullptr);
    explicit InvertFilter(DImg* const orgImage, QObject* const parent = nullptr);

    // Constructor for slave mode: execute immediately in current thread with specified master filter
    explicit InvertFilter(DImgThreadedFilter* const parentFilter, const DImg& orgImage, DImg& destImage,
                          int progressBegin = 0, int progressEnd = 100);

    ~InvertFilter() override;



    static QString          FilterIdentifier()
    {
        return QLatin1String("digikam:InvertFilter");
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

    void                    readParameters(const FilterAction& action)        override;

    QString         filterIdentifier()                                  const override
    {
        return FilterIdentifier();
    }

    FilterAction    filterAction()                                            override;

private:

    void filterImage()                                                        override;
};

} // namespace Digikam

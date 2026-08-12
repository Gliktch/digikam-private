/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-25-02
 * Description : Curves image filter
 *
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2010      by Martin Klapetek <martin dot klapetek at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QPolygon>

// Local includes

#include "digikam_export.h"
#include "dimgthreadedfilter.h"
#include "digikam_globals.h"
#include "imagecurves.h"

namespace Digikam
{

class DImg;

class DIGIKAM_EXPORT CurvesFilter : public DImgThreadedFilter
{
    Q_OBJECT

public:

    explicit CurvesFilter(QObject* const parent = nullptr);
    explicit CurvesFilter(DImg* const orgImage,
                          QObject* const parent = nullptr,
                          const CurvesContainer& settings = CurvesContainer());
    explicit CurvesFilter(const CurvesContainer& settings,
                          DImgThreadedFilter* const master,
                          const DImg& orgImage,
                          DImg& destImage,
                          int progressBegin = 0,
                          int progressEnd = 100);
    ~CurvesFilter()                                                 override;

    static QString FilterIdentifier()
    {
        return QLatin1String("digikam:CurvesFilter");
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

    QString filterIdentifier()                               const override
    {
        return FilterIdentifier();
    }

    FilterAction filterAction()                                    override;
    void readParameters(const FilterAction& action)                override;

private:

    void filterImage()                                             override;

private:

    CurvesContainer m_settings;
};

} // namespace Digikam

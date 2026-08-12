/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-05-25
 * Description : lens distortion algorithm.
 *
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2001-2003 by David Hodson <hodsond@acm.org>
 * SPDX-FileCopyrightText: 2010      by Martin Klapetek <martin dot klapetek at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "dimgthreadedfilter.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT LensDistortionFilter : public DImgThreadedFilter
{
    Q_OBJECT

public:

    explicit LensDistortionFilter(QObject* const parent = nullptr);
    explicit LensDistortionFilter(DImg* const orgImage, QObject* const parent = nullptr, double main = 0.0,
                                  double edge = 0.0, double rescale = 0.0, double brighten = 0.0,
                                  int center_x = 0, int center_y = 0);

    ~LensDistortionFilter()                                                   override;

    static QString          FilterIdentifier()
    {
        return QLatin1String("digikam:LensDistortionFilter");
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

private:

    int    m_centre_x = 0;
    int    m_centre_y = 0;

    double m_main     = 0.0;
    double m_edge     = 0.0;
    double m_rescale  = 0.0;
    double m_brighten = 0.0;
};

} // namespace Digikam

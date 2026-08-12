/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-05-25
 * Description : Antivignetting threaded image filter.
 *
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

class DIGIKAM_EXPORT AntiVignettingContainer
{

public:

    AntiVignettingContainer()  = default;
    ~AntiVignettingContainer() = default;

public:

    bool   addvignetting    = true;

    double density          = 2.0;
    double power            = 1.0;
    double innerradius      = 1.0;
    double outerradius      = 1.0;
    double xshift           = 0.0;
    double yshift           = 0.0;
};

// -----------------------------------------------------------------------------------------------

class DIGIKAM_EXPORT AntiVignettingFilter : public DImgThreadedFilter
{
    Q_OBJECT

public:

    explicit AntiVignettingFilter(QObject* const parent = nullptr);
    explicit AntiVignettingFilter(DImg* const orgImage, QObject* const parent = nullptr,
                                  const AntiVignettingContainer& settings = AntiVignettingContainer());

    ~AntiVignettingFilter()                                override;

    static QString FilterIdentifier()
    {
        return QLatin1String("digikam:AntiVignettingFilter");
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

    void readParameters(const FilterAction& action)        override;

    QString filterIdentifier()                       const override
    {
        return FilterIdentifier();
    }

    FilterAction filterAction()                            override;

private:

    void           filterImage()                           override;

    double         hypotenuse(double x, double y);
    uchar          clamp8bits(double x);
    unsigned short clamp16bits(double x);
    double         attenuation(double r1, double r2, double distcenter);
    double         real_attenuation(double r1, double r2, double distcenter);

private:

    AntiVignettingContainer m_settings;
};

} // namespace Digikam

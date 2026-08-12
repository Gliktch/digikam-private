/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-03-06
 * Description : Hue/Saturation/Lightness image filter.
 *
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2010      by Martin Klapetek <martin dot klapetek at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 * ============================================================ */

#pragma once

// Local includes

#include "digikam_export.h"
#include "dimgthreadedfilter.h"
#include "digikam_globals.h"

namespace Digikam
{

class DImg;

class DIGIKAM_EXPORT HSLContainer
{

public:

    HSLContainer()  = default;
    ~HSLContainer() = default;

public:

    double hue          = 0.0;
    double saturation   = 0.0;
    double vibrance     = 0.0;
    double lightness    = 0.0;
};

// -----------------------------------------------------------------------------------------------

class DIGIKAM_EXPORT HSLFilter : public DImgThreadedFilter
{
    Q_OBJECT

public:

    explicit HSLFilter(QObject* const parent = nullptr);
    explicit HSLFilter(DImg* const orgImage, QObject* const parent = nullptr,
                       const HSLContainer& settings = HSLContainer());
    ~HSLFilter()                                           override;

    static QString FilterIdentifier()
    {
        return QLatin1String("digikam:HSLFilter");
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

    QString filterIdentifier()                       const override
    {
        return FilterIdentifier();
    }

    FilterAction filterAction()                            override;

    void readParameters(const FilterAction& action)        override;

private:

    void filterImage()                                     override;

    void reset();
    void setHue(double val);
    void setSaturation(double val);
    void setLightness(double val);
    void applyHSL(const DImg& image);
    int  vibranceBias(double sat, double hue, double vib, bool sixteenbit);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam

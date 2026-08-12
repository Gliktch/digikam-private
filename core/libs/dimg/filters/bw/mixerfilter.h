/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-24-01
 * Description : Channels mixer filter
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
#include "digikam_globals.h"

namespace Digikam
{

class DImg;

class DIGIKAM_EXPORT MixerContainer
{

public:

    MixerContainer()  = default;
    ~MixerContainer() = default;

public:

    bool   bPreserveLum     = true;
    bool   bMonochrome      = false;

    // Standard settings.
    double redRedGain       = 1.0;
    double redGreenGain     = 0.0;
    double redBlueGain      = 0.0;
    double greenRedGain     = 0.0;
    double greenGreenGain   = 1.0;
    double greenBlueGain    = 0.0;
    double blueRedGain      = 0.0;
    double blueGreenGain    = 0.0;
    double blueBlueGain     = 1.0;

    // Monochrome settings.
    double blackRedGain     = 1.0;
    double blackGreenGain   = 0.0;
    double blackBlueGain    = 0.0;
};

// -----------------------------------------------------------------------------------------------

class DIGIKAM_EXPORT MixerFilter : public DImgThreadedFilter
{
    Q_OBJECT

public:

    explicit MixerFilter(QObject* const parent = nullptr);
    explicit MixerFilter(DImg* const orgImage,
                         QObject* const parent = nullptr,
                         const MixerContainer& settings = MixerContainer());
    ~MixerFilter()                                         override;

    static QString FilterIdentifier()
    {
        return QLatin1String("digikam:MixerFilter");
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

    inline double CalculateNorm(double RedGain, double GreenGain, double BlueGain, bool bPreserveLum);

    inline unsigned short MixPixel(double RedGain, double GreenGain, double BlueGain,
                                   unsigned short R, unsigned short G, unsigned short B, bool sixteenBit,
                                   double Norm);

private:

    MixerContainer m_settings;
};

} // namespace Digikam

/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-05-25
 * Description : filter to add Film Grain to image.
 *
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2005-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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
#include "dcolor.h"

namespace Digikam
{

class DIGIKAM_EXPORT FilmGrainContainer
{

public:

    FilmGrainContainer()  = default;
    ~FilmGrainContainer() = default;

    bool isDirty() const
    {
        return (addLuminanceNoise || addChrominanceBlueNoise || addChrominanceRedNoise);
    };

public:

    int  grainSize                  = 1;
    bool photoDistribution          = false;

    bool addLuminanceNoise          = true;
    int  lumaIntensity              = 25;
    int  lumaShadows                = -100;
    int  lumaMidtones               = 0;
    int  lumaHighlights             = -100;

    bool addChrominanceBlueNoise    = false;
    int  chromaBlueIntensity        = 25;
    int  chromaBlueShadows          = -100;
    int  chromaBlueMidtones         = 0;
    int  chromaBlueHighlights       = -100;

    bool addChrominanceRedNoise     = false;
    int  chromaRedIntensity         = 25;
    int  chromaRedShadows           = -100;
    int  chromaRedMidtones          = 0;
    int  chromaRedHighlights        = -100;
};

// -----------------------------------------------------------------------------------------------

class DIGIKAM_EXPORT FilmGrainFilter : public DImgThreadedFilter
{
    Q_OBJECT

public:

    explicit FilmGrainFilter(QObject* const parent = nullptr);
    explicit FilmGrainFilter(DImg* const orgImage, QObject* const parent = nullptr,
                             const FilmGrainContainer& settings = FilmGrainContainer());

    /**
     * @brief: constructor for slave mode: execute immediately in current thread with specified master filter.
     */
    explicit FilmGrainFilter(DImgThreadedFilter* const parentFilter, const DImg& orgImage, const DImg& destImage,
                             int progressBegin = 0, int progressEnd = 100,
                             const FilmGrainContainer& settings = FilmGrainContainer());
    ~FilmGrainFilter()                                                        override;


    static QString          FilterIdentifier()
    {
        return QLatin1String("digikam:FilmGrainFilter");
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

    void filmgrainMultithreaded(uint start, uint stop);

    inline void computeNoiseSettings(const DColor& col,
                                     double& luRange, double& luNoise,
                                     double& cbRange, double& cbNoise,
                                     double& crRange, double& crNoise);
    inline double interpolate(int shadows, int midtones, int highlights, const DColor& col);
    inline double randomizeUniform(double range);
    inline double randomizeGauss(double sigma);
    inline double randomizePoisson(double lambda);
    inline void   adjustYCbCr(DColor& col, double range, double nRand, int channel);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam

/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-03-06
 * Description : black and white image filter.
 *
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include "bcgfilter.h"
#include "curvesfilter.h"
#include "tonalityfilter.h"

namespace Digikam
{

class DImg;

class DIGIKAM_EXPORT BWSepiaContainer
{

public:

    enum BlackWhiteConversionType
    {
        BWNoFilter = 0,       ///< B&W filter to the front of lens.
        BWGreenFilter,
        BWOrangeFilter,
        BWRedFilter,
        BWYellowFilter,
        BWYellowGreenFilter,
        BWBlueFilter,

        BWGeneric,            ///< B&W film simulation.
        BWAgfa200X,
        BWAgfapan25,
        BWAgfapan100,
        BWAgfapan400,
        BWIlfordDelta100,
        BWIlfordDelta400,
        BWIlfordDelta400Pro3200,
        BWIlfordFP4,
        BWIlfordHP5,
        BWIlfordPanF,
        BWIlfordXP2Super,
        BWKodakTmax100,
        BWKodakTmax400,
        BWKodakTriX,

        BWIlfordSFX200,       ///< Infrared film simulation.
        BWIlfordSFX400,
        BWIlfordSFX800,

        BWNoTone,             ///< Chemical color tone filter.
        BWSepiaTone,
        BWBrownTone,
        BWColdTone,
        BWSeleniumTone,
        BWPlatinumTone,
        BWGreenTone,

        // Filter version 2
        BWKodakHIE            ///< Infrared film simulation.
    };

public:

    BWSepiaContainer() = default;

    explicit BWSepiaContainer(int ptype)
        : preview    (true),
          previewType(ptype)
    {
    }

    BWSepiaContainer(int ptype, const CurvesContainer& container)
        : preview    (true),
          previewType(ptype),
          curvesPrm  (container)
    {
    }

    ~BWSepiaContainer() = default;

public:

    bool            preview     = false;

    int             previewType = BWGeneric;
    int             filmType    = BWGeneric;
    int             filterType  = BWNoFilter;
    int             toneType    = BWNoTone;

    double          strength    = 1.0;

    CurvesContainer curvesPrm;

    BCGContainer    bcgPrm;
};

// -----------------------------------------------------------------------------------------------

class DIGIKAM_EXPORT BWSepiaFilter : public DImgThreadedFilter
{
    Q_OBJECT

public:

    explicit BWSepiaFilter(QObject* const parent = nullptr);
    explicit BWSepiaFilter(DImg* orgImage,
                           QObject* const parent = nullptr,
                           const BWSepiaContainer& settings = BWSepiaContainer());
    ~BWSepiaFilter()                                        override;

    static QString FilterIdentifier()
    {
        return QLatin1String("digikam:BWSepiaFilter");
    }

    static QString DisplayableName();

    static QList<int> SupportedVersions()
    {
        return QList<int>() << 1 << 2;
    }

    static int CurrentVersion()
    {
        return 2;
    }

    QString filterIdentifier()                        const override
    {
        return FilterIdentifier();
    }

    FilterAction filterAction()                             override;
    void readParameters(const FilterAction& action)         override;

private:

    void filterImage()                                      override;

    DImg getThumbnailForEffect(const DImg& img);

    void blackAndWhiteConversion(DImg& img, int type);
    void applyChannelMixer(DImg& img);
    void applyInfraredFilter(DImg& img, int sensibility);
    void applyToneFilter(DImg& img, TonalityContainer& settings);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam

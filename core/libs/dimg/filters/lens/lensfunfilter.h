/* ============================================================
 *
 * Date        : 2008-02-10
 * Description : a tool to fix automatically camera lens aberrations
 *
 * SPDX-FileCopyrightText: 2008      by Adrian Schroeter <adrian at suse dot de>
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "digikam_config.h"
#include "dimgthreadedfilter.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT LensFunContainer
{

public:

    LensFunContainer()  = default;
    ~LensFunContainer() = default;

public:

    bool      filterCCA         = true;       ///< Chromatic Aberration Corrections
    bool      filterVIG         = true;       ///< Vignetting Corrections
    bool      filterDST         = true;       ///< Distortion Corrections
    bool      filterGEO         = true;       ///< Geometry Corrections

    double    cropFactor        = -1.0;
    double    focalLength       = -1.0;
    double    aperture          = -1.0;
    double    subjectDistance   = -1.0;

    QString   cameraMake;
    QString   cameraModel;
    QString   lensModel;
};

// -----------------------------------------------------------------

class DIGIKAM_EXPORT LensFunFilter : public DImgThreadedFilter
{
    Q_OBJECT

public:

    explicit LensFunFilter(QObject* const parent = nullptr);
    explicit LensFunFilter(DImg* const origImage,
                           QObject* const parent,
                           const LensFunContainer& settings);
    ~LensFunFilter()                                       override;

    bool registerSettingsToXmp(MetaEngineData& data) const;
    void readParameters(const FilterAction& action)        override;

    static QString FilterIdentifier()
    {
        return QLatin1String("digikam:LensFunFilter");
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

    QString filterIdentifier()                       const override
    {
        return FilterIdentifier();
    }

    FilterAction filterAction()                            override;

private:

    void filterImage()                                     override;
    void filterCCAMultithreaded(uint start, uint stop);
    void filterVIGMultithreaded(uint start, uint stop);
    void filterDSTMultithreaded(uint start, uint stop);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam

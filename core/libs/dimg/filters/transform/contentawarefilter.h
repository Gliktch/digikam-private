/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-02-01
 * Description : Content aware resizer class.
 *
 * SPDX-FileCopyrightText: 2009      by Julien Pontabry <julien dot pontabry at ulp dot u-strasbg dot fr>
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2010      by Martin Klapetek <martin dot klapetek at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QImage>

// Local includes

#include "digikam_config.h"
#include "dcolor.h"
#include "digikam_export.h"
#include "dimgthreadedfilter.h"

namespace Digikam
{

class DIGIKAM_EXPORT ContentAwareContainer
{

public:

    enum EnergyFunction
    {
        GradientNorm = 0,
        SumOfAbsoluteValues,
        XAbsoluteValue,
        LumaGradientNorm,
        LumaSumOfAbsoluteValues,
        LumaXAbsoluteValue
    };

public:

    ContentAwareContainer()  = default;
    ~ContentAwareContainer() = default;

public:

    bool            preserve_skin_tones = false;

    uint            width               = 0;
    uint            height              = 0;

    int             step                = 1;
    int             side_switch_freq    = 4;

    double          rigidity            = 0.0;

    QImage          mask;

    EnergyFunction  func                = GradientNorm;
    Qt::Orientation resize_order        = Qt::Horizontal;
};

// -----------------------------------------------------------------------------------------

class DIGIKAM_EXPORT ContentAwareFilter : public DImgThreadedFilter
{
    Q_OBJECT

public:

    explicit ContentAwareFilter(QObject* const parent = nullptr);
    explicit ContentAwareFilter(DImg* const orgImage,
                                QObject* const parent = nullptr,
                                const ContentAwareContainer& settings = ContentAwareContainer());
    ~ContentAwareFilter()                                                     override;

    void progressCallback(int progress);

    static QString          FilterIdentifier()
    {
        return QLatin1String("digikam:ContentAwareFilter");
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

    void cancelFilter()                                                       override;
    void filterImage()                                                        override;

    void        buildBias(const QImage& mask);
    void        buildSkinToneBias();
    inline bool isSkinTone(const DColor& c);
    void        getEnergyImage();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam

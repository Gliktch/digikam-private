/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-03-06
 * Description : a Brightness/Contrast/Gamma image filter.
 *
 * SPDX-FileCopyrightText: 2005      by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2010      by Martin Klapetek <martin dot klapetek at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "bcgcontainer.h"
#include "digikam_export.h"
#include "dimgthreadedfilter.h"
#include "digikam_globals.h"

namespace Digikam
{

class DImg;

class DIGIKAM_EXPORT BCGFilter : public DImgThreadedFilter
{
    Q_OBJECT

public:

    explicit BCGFilter(QObject* const parent = nullptr);
    explicit BCGFilter(DImg* const orgImage,
                       QObject* const parent = nullptr,
                       const BCGContainer& settings = BCGContainer());
    explicit BCGFilter(const BCGContainer& settings,
                       DImgThreadedFilter* const master,
                       const DImg& orgImage,
                       const DImg& destImage,
                       int progressBegin = 0,
                       int progressEnd = 100);
    ~BCGFilter()                                                  override;

    static QString FilterIdentifier()
    {
        return QLatin1String("digikam:BCGFilter");
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

    QString filterIdentifier()                              const override
    {
        return FilterIdentifier();
    }

    FilterAction filterAction()                                   override;

    void readParameters(const FilterAction& action)               override;

private:

    void filterImage()                                            override;

    void reset();
    void setGamma(double val);
    void setBrightness(double val);
    void setContrast(double val);
    void applyBCG(const DImg& image);
    void applyBCG(uchar* const bits, uint width, uint height, bool sixteenBits);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
